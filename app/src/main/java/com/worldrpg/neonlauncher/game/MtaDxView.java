package com.worldrpg.neonlauncher.game;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.PorterDuffXfermode;
import android.graphics.Path;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.view.View;

import java.util.HashMap;

/** Draws the rectangles, text, and images produced by client resource scripts. */
public final class MtaDxView extends View {
    private String frame = "";
    // Desktop dxDrawRectangle is a sharp pixel quad. Anti-alias insets the edge
    // by a fraction of a pixel, so borders look pulled inside the box.
    private final Paint fill = new Paint();
    private final Paint image = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.FILTER_BITMAP_FLAG);
    private final TextPaint text = new TextPaint(Paint.ANTI_ALIAS_FLAG);
    private final HashMap<String, Bitmap> images = new HashMap<>();
    private final Rect source = new Rect();
    private final RectF destination = new RectF();
    // dxDrawLine and the line and point primitives.
    private final Paint stroke = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Path triangle = new Path();
    // Same font as the text paint above, for dxGetTextWidth / dxGetTextSize (any thread).
    private static final TextPaint MEASURE = new TextPaint(Paint.ANTI_ALIAS_FLAG);
    static {
        MEASURE.setTypeface(Typeface.DEFAULT_BOLD);
    }

    // dxCreateFont files, keyed by path and weight. A file that does not load is kept as the
    // default font so it is not read again every frame.
    private static final HashMap<String, Typeface> FONTS = new HashMap<>();

    private static Typeface typefaceFor(String fontFile, boolean bold) {
        if (fontFile == null || fontFile.isEmpty()) return Typeface.DEFAULT_BOLD;
        String key = (bold ? "b:" : "n:") + fontFile;
        synchronized (FONTS) {
            Typeface cached = FONTS.get(key);
            if (cached != null) return cached;
            Typeface loaded;
            try {
                loaded = Typeface.createFromFile(fontFile);
                if (bold) loaded = Typeface.create(loaded, Typeface.BOLD);
            } catch (RuntimeException error) {
                loaded = Typeface.DEFAULT_BOLD;
            }
            FONTS.put(key, loaded);
            return loaded;
        }
    }

    /**
     * Width of one line of text at this pixel size, in the font the overlay draws with, or in
     * the dxCreateFont file when one is given.
     */
    public static float measureLine(String line, float pixelSize, String fontFile, boolean bold) {
        if (line == null || line.isEmpty() || !(pixelSize > 0f)) return 0f;
        Typeface typeface = typefaceFor(fontFile, bold);
        synchronized (MEASURE) {
            MEASURE.setTypeface(typeface);
            MEASURE.setTextSize(pixelSize);
            return MEASURE.measureText(line);
        }
    }

    // Text size of scale 1: 15 px for the default font, the font height after an "F" line.
    private float textBase = 15f;

    public MtaDxView(android.content.Context context) {
        super(context);
        setClickable(false);
        text.setTypeface(Typeface.DEFAULT_BOLD);
        text.setShadowLayer(2f, 1f, 1f, 0xCC000000);
    }

    /** Receives touches while a script has the cursor shown. action: 0 down, 1 up, 2 move. */
    public interface CursorListener {
        void onCursor(int action, float x, float y);
    }

    private CursorListener cursorListener;
    private boolean cursorShown;

    public void setCursorListener(CursorListener listener) {
        cursorListener = listener;
    }

    // Desktop: with showCursor(true) the mouse drives the interface and the game gets no input.
    // Here the overlay takes the touch and reports it at the coordinates the frame is drawn in.
    @Override
    public boolean onTouchEvent(android.view.MotionEvent event) {
        if (!cursorShown || cursorListener == null) return false;
        switch (event.getActionMasked()) {
            case android.view.MotionEvent.ACTION_DOWN:
                cursorListener.onCursor(0, event.getX(), event.getY());
                break;
            case android.view.MotionEvent.ACTION_MOVE:
                cursorListener.onCursor(2, event.getX(), event.getY());
                break;
            case android.view.MotionEvent.ACTION_UP:
            case android.view.MotionEvent.ACTION_CANCEL:
                cursorListener.onCursor(1, event.getX(), event.getY());
                break;
            default:
                break;
        }
        return true;
    }

    // Render-target sections of frames that were replaced before they were drawn. The view
    // draws only the newest frame, but a script may fill a render target once and never again,
    // so those sections are kept and run at the next draw.
    private final StringBuilder carried = new StringBuilder();
    private boolean frameDrawn = true;
    private static final int MAX_CARRIED = 200000;

    // The lines from each "G id ..." up to the bare "G" that ends it.
    private static void collectTargetSections(String commands, StringBuilder out) {
        boolean inside = false;
        for (String line : commands.split("\n")) {
            if (line.startsWith("G ")) inside = true;
            if (inside) out.append(line).append('\n');
            if (line.equals("G")) inside = false;
        }
        if (inside) out.append("G\n");
    }

    public void setFrame(String commands) {
        String next = commands == null ? "" : commands;
        if (!frameDrawn && frame.contains("G ") && carried.length() < MAX_CARRIED) collectTargetSections(frame, carried);
        // "D id" frees script bitmaps. It is handled here, not in onDraw, so it is never skipped.
        if (next.contains("D ")) {
            for (String line : next.split("\n")) {
                if (line.startsWith("D ")) freeTarget(line.substring(2).trim());
            }
        }
        frameDrawn = false;
        frame = next;
        // A first line "C" means the cursor is showing (LuaVMStub::pumpDx).
        cursorShown = frame.startsWith("C\n") || frame.equals("C");
        setVisibility(frame.isEmpty() ? GONE : VISIBLE);
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        frameDrawn = true;
        if (frame.isEmpty() && carried.length() == 0) return;
        String[] lines = (carried.length() == 0 ? frame : carried + frame).split("\n");
        carried.setLength(0);
        // Every frame starts in "blend"; a "B" line at the top carries a mode set in an earlier frame.
        setBlendMode(0);
        selectFont("");
        // The screen, or the bitmap of the render target selected by a "G" line.
        Canvas screen = canvas;
        for (String line : lines) {
            try {
                canvas = drawLineOfFrame(canvas, screen, line);
            } catch (RuntimeException error) {
                // One bad draw line (a bitmap freed by the script thread, a vertex count Canvas
                // refuses) must not take the Activity down.
            }
        }
        setBlendMode(0);
        selectFont("");
    }

    private Canvas drawLineOfFrame(Canvas canvas, Canvas screen, String line) {
        {
            if (line.startsWith("R ")) drawRect(canvas, line.substring(2));
            else if (line.startsWith("T ")) drawText(canvas, line.substring(2));
            else if (line.startsWith("I ")) drawImage(canvas, line.substring(2));
            else if (line.startsWith("L ")) drawLine(canvas, line.substring(2));
            else if (line.startsWith("P ")) drawPrimitive(canvas, line.substring(2));
            else if (line.startsWith("B ")) setBlendMode((int) l(line.substring(2).trim()));
            else if (line.startsWith("M ")) drawMaterialPrimitive(canvas, line.substring(2));
            else if (line.startsWith("F")) selectFont(line.length() > 2 ? line.substring(2) : "");
            else if (line.startsWith("G")) return selectTarget(screen, line.length() > 2 ? line.substring(2) : "");
        }
        return canvas;
    }

    // dxSetBlendMode: 0 blend, 1 add, 2 modulate_add, 3 overwrite (desktop EBlendModeType).
    // modulate_add is for images with premultiplied alpha; Canvas already blends that way,
    // so it draws as blend.
    private static final PorterDuffXfermode XFER_ADD = new PorterDuffXfermode(PorterDuff.Mode.ADD);
    private static final PorterDuffXfermode XFER_OVERWRITE = new PorterDuffXfermode(PorterDuff.Mode.SRC);

    private void setBlendMode(int mode) {
        PorterDuffXfermode xfer = mode == 1 ? XFER_ADD : mode == 3 ? XFER_OVERWRITE : null;
        fill.setXfermode(xfer);
        image.setXfermode(xfer);
        text.setXfermode(xfer);
        stroke.setXfermode(xfer);
    }

    // Bitmaps the scripts own, by the name dxDrawImage uses for them: "rt:<id>" for a render
    // target and "tx:<id>" for a texture made from pixels. The VM thread reads and writes them
    // through the static pixel calls below, so every access holds the map's lock.
    private static final HashMap<String, Bitmap> MEMORY = new HashMap<>();
    private final HashMap<String, Canvas> targetCanvases = new HashMap<>();
    // 4096 x 4096, the largest bitmap the pixel calls and render targets take.
    private static final long MAX_PIXELS = 4096L * 4096L;

    private static int[] pixelsOf(Bitmap bitmap) {
        int width = bitmap.getWidth(), height = bitmap.getHeight();
        if (width < 1 || height < 1 || (long) width * height > MAX_PIXELS) return null;
        int[] out = new int[2 + width * height];
        out[0] = width;
        out[1] = height;
        bitmap.getPixels(out, 2, width, 0, 0, width, height);
        return out;
    }

    private static BitmapFactory.Options decodeOptions() {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return options;
    }

    /** {width, height, 0xAARRGGBB pixels...} of an image file or a script bitmap, or null. */
    public static int[] loadPixels(String source) {
        if (source == null || source.isEmpty()) return null;
        try {
            if (source.startsWith("rt:") || source.startsWith("tx:")) {
                synchronized (MEMORY) {
                    Bitmap bitmap = MEMORY.get(source);
                    return bitmap == null || bitmap.isRecycled() ? null : pixelsOf(bitmap);
                }
            }
            Bitmap decoded = BitmapFactory.decodeFile(source, decodeOptions());
            if (decoded == null) return null;
            int[] out = pixelsOf(decoded);
            decoded.recycle();
            return out;
        } catch (RuntimeException | OutOfMemoryError error) {
            return null;
        }
    }

    /** {width, height, pixels...} of PNG or JPEG bytes, or null. */
    public static int[] decodePixels(byte[] data) {
        if (data == null || data.length == 0) return null;
        try {
            Bitmap decoded = BitmapFactory.decodeByteArray(data, 0, data.length, decodeOptions());
            if (decoded == null) return null;
            int[] out = pixelsOf(decoded);
            decoded.recycle();
            return out;
        } catch (RuntimeException | OutOfMemoryError error) {
            return null;
        }
    }

    /** PNG or JPEG bytes of the pixels, or null. */
    public static byte[] encodePixels(int[] argb, int width, int height, boolean jpeg, int quality) {
        if (argb == null || width < 1 || height < 1 || (long) width * height != argb.length) return null;
        try {
            Bitmap bitmap = Bitmap.createBitmap(argb, width, height, Bitmap.Config.ARGB_8888);
            java.io.ByteArrayOutputStream out = new java.io.ByteArrayOutputStream();
            boolean ok = bitmap.compress(jpeg ? Bitmap.CompressFormat.JPEG : Bitmap.CompressFormat.PNG,
                    Math.max(0, Math.min(100, quality)), out);
            bitmap.recycle();
            return ok ? out.toByteArray() : null;
        } catch (RuntimeException | OutOfMemoryError error) {
            return null;
        }
    }

    /**
     * Draws an SVG document into the script bitmap of this name (svgCreate, svgSetDocumentXML,
     * svgSetSize), scaled to the size as desktop does. 1 drawn, 0 the document is not readable
     * as SVG, -1 the bitmap could not be made.
     */
    public static int renderSvg(String name, byte[] xml, int width, int height) {
        if (name == null || xml == null || width < 1 || height < 1 || (long) width * height > MAX_PIXELS) return -1;
        com.caverock.androidsvg.SVG svg;
        try {
            svg = com.caverock.androidsvg.SVG.getFromString(new String(xml, java.nio.charset.StandardCharsets.UTF_8));
        } catch (com.caverock.androidsvg.SVGParseException | RuntimeException error) {
            return 0;
        }
        try {
            // Without a viewBox the document would be drawn at its own size and cut off.
            if (svg.getDocumentViewBox() == null) {
                float ownWidth = svg.getDocumentWidth(), ownHeight = svg.getDocumentHeight();
                if (ownWidth > 0f && ownHeight > 0f) svg.setDocumentViewBox(0f, 0f, ownWidth, ownHeight);
            }
            svg.setDocumentWidth(width);
            svg.setDocumentHeight(height);
            Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            svg.renderToCanvas(new Canvas(bitmap));
            // The old bitmap is left to the garbage collector: the UI thread may be drawing it now,
            // and drawing a recycled bitmap throws.
            synchronized (MEMORY) {
                MEMORY.put(name, bitmap);
            }
            return 1;
        } catch (RuntimeException error) {
            return 0;
        } catch (OutOfMemoryError error) {
            return -1;
        }
    }

    // Screen sources with a copy in flight, so a script that updates every frame queues one at a time.
    private static final java.util.HashSet<String> COPYING = new java.util.HashSet<>();

    /**
     * dxUpdateScreenSource: copies the game surface, scaled, into the script bitmap of this name.
     * Call on the UI thread. The copy is asynchronous (PixelCopy).
     */
    public static void copyScreen(android.view.SurfaceView surface, String name, int width, int height) {
        if (surface == null || name == null || width < 1 || height < 1 || (long) width * height > MAX_PIXELS) return;
        if (surface.getHolder() == null || surface.getHolder().getSurface() == null
                || !surface.getHolder().getSurface().isValid()) {
            return;
        }
        final Bitmap bitmap;
        synchronized (MEMORY) {
            if (COPYING.contains(name)) return;
            Bitmap existing = MEMORY.get(name);
            if (existing == null || existing.isRecycled() || existing.getWidth() != width || existing.getHeight() != height) {
                try {
                    existing = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                } catch (OutOfMemoryError error) {
                    return;
                }
                Bitmap old = MEMORY.put(name, existing);
                if (old != null) old.recycle();
            }
            bitmap = existing;
            COPYING.add(name);
        }
        try {
            android.view.PixelCopy.request(surface, bitmap, result -> {
                synchronized (MEMORY) {
                    COPYING.remove(name);
                }
            }, new android.os.Handler(android.os.Looper.getMainLooper()));
        } catch (RuntimeException error) {
            synchronized (MEMORY) {
                COPYING.remove(name);
            }
        }
    }

    /** Creates the script bitmap of this name, or overwrites it when it has the same size. */
    public static boolean storePixels(String name, int[] argb, int width, int height) {
        if (name == null || argb == null || width < 1 || height < 1 || (long) width * height != argb.length
                || (long) width * height > MAX_PIXELS) {
            return false;
        }
        try {
            synchronized (MEMORY) {
                Bitmap bitmap = MEMORY.get(name);
                // A render target keeps its bitmap: its Canvas draws into that object.
                if (bitmap == null || bitmap.isRecycled() || bitmap.getWidth() != width || bitmap.getHeight() != height) {
                    if (name.startsWith("rt:") && bitmap != null) return false;
                    bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                    MEMORY.put(name, bitmap);
                }
                bitmap.setPixels(argb, 0, width, 0, 0, width, height);
            }
            return true;
        } catch (RuntimeException | OutOfMemoryError error) {
            return false;
        }
    }

    // "id width height alpha clear" from dxSetRenderTarget: the lines that follow draw into that
    // target's bitmap, which keeps its picture between frames. An empty body is the screen again.
    // Where the draws of a target that could not be made go, so they do not land on the screen.
    private final Canvas discard = new Canvas(Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888));

    private Canvas selectTarget(Canvas screen, String body) {
        String[] p = body.split(" ");
        if (p.length < 5) return screen;
        String name = "rt:" + p[0];
        synchronized (MEMORY) {
            Bitmap bitmap = MEMORY.get(name);
            Canvas target = targetCanvases.get(name);
            if (bitmap == null || bitmap.isRecycled()) {
                int width = (int) l(p[1]), height = (int) l(p[2]);
                if (width < 1 || height < 1 || width > 4096 || height > 4096) return discard;
                try {
                    bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                } catch (OutOfMemoryError error) {
                    return discard;
                }
                bitmap.eraseColor("1".equals(p[3]) ? Color.TRANSPARENT : Color.BLACK);
                MEMORY.put(name, bitmap);
                target = null;
            }
            // dxSetTexturePixels may have made the bitmap before the first draw into it.
            if (target == null || !bitmap.isMutable()) {
                target = new Canvas(bitmap);
                targetCanvases.put(name, target);
            }
            if ("1".equals(p[4])) bitmap.eraseColor("1".equals(p[3]) ? Color.TRANSPARENT : Color.BLACK);
            return target;
        }
    }

    // "D id": the render target or pixel texture of this id was destroyed.
    private void freeTarget(String id) {
        targetCanvases.remove("rt:" + id);
        synchronized (MEMORY) {
            for (String name : new String[] {"rt:" + id, "tx:" + id}) {
                Bitmap bitmap = MEMORY.remove(name);
                if (bitmap != null) bitmap.recycle();
            }
        }
    }

    // "height bold file" selects a dxCreateFont font for the text lines that follow; an empty body
    // returns to the default font.
    private void selectFont(String body) {
        String[] p = body.split(" ", 3);
        if (p.length < 3 || p[2].isEmpty()) {
            text.setTypeface(Typeface.DEFAULT_BOLD);
            textBase = 15f;
            return;
        }
        text.setTypeface(typefaceFor(p[2], "1".equals(p[1])));
        textBase = Math.max(1f, f(p[0]));
    }

    // "M type count x y color u v ... file" from dxDrawMaterialPrimitive: triangles textured with
    // the image, u and v running 0-1 across it. Needs the hardware drawVertices of API 29; older
    // phones draw nothing. Point and line types have no textured form on Canvas and are skipped.
    private void drawMaterialPrimitive(Canvas canvas, String body) {
        String[] head = body.split(" ", 3);
        if (head.length < 3) return;
        int type = (int) l(head[0]);
        int count = (int) l(head[1]);
        if (type < 4 || type > 6 || count < 3 || count > 4096) return;
        if (android.os.Build.VERSION.SDK_INT < 29) return;
        String[] p = head[2].split(" ", count * 5 + 1);
        if (p.length < count * 5 + 1) return;
        Bitmap bitmap = bitmapFor(p[count * 5]);
        if (bitmap == null) return;
        float[] xy = new float[count * 2];
        float[] uv = new float[count * 2];
        int[] colors = new int[count * 2];
        for (int i = 0; i < count; i++) {
            xy[i * 2] = f(p[i * 5]);
            xy[i * 2 + 1] = f(p[i * 5 + 1]);
            colors[i] = (int) l(p[i * 5 + 2]);
            uv[i * 2] = f(p[i * 5 + 3]) * bitmap.getWidth();
            uv[i * 2 + 1] = f(p[i * 5 + 4]) * bitmap.getHeight();
        }
        Canvas.VertexMode mode = type == 4 ? Canvas.VertexMode.TRIANGLES
                : type == 5 ? Canvas.VertexMode.TRIANGLE_STRIP : Canvas.VertexMode.TRIANGLE_FAN;
        int used = type == 4 ? count - count % 3 : count;
        fill.setColor(Color.WHITE);
        fill.setShader(new android.graphics.BitmapShader(
                bitmap, android.graphics.Shader.TileMode.REPEAT, android.graphics.Shader.TileMode.REPEAT));
        try {
            canvas.drawVertices(mode, used * 2, xy, 0, uv, 0, colors, 0, null, 0, 0, fill);
        } finally {
            // The outer frame handler catches a failed draw. Always release this
            // shader so the next rectangle/GUI element does not inherit the texture.
            fill.setShader(null);
        }
    }

    // "L x1 y1 x2 y2 color width" from dxDrawLine.
    private void drawLine(Canvas canvas, String body) {
        String[] p = body.split(" ");
        if (p.length < 6) return;
        stroke.setStyle(Paint.Style.STROKE);
        stroke.setShader(null);
        stroke.setColor((int) l(p[4]));
        stroke.setStrokeWidth(Math.max(1f, f(p[5])));
        canvas.drawLine(f(p[0]), f(p[1]), f(p[2]), f(p[3]), stroke);
    }

    // "P type count x y color ..." from dxDrawPrimitive and dxDrawCircle. type is the D3D
    // primitive type: 1 point list, 2 line list, 3 line strip, 4 triangle list, 5 triangle strip,
    // 6 triangle fan. Triangles are filled with per-vertex colours where Canvas.drawVertices is
    // hardware accelerated (API 29), otherwise each triangle takes the colour of its first vertex.
    // Lines interpolate their endpoint colours with a linear gradient.
    private void drawPrimitive(Canvas canvas, String body) {
        String[] p = body.split(" ");
        if (p.length < 2) return;
        int type = (int) l(p[0]);
        int count = (int) l(p[1]);
        if (count < 1 || count > 4096 || p.length < 2 + count * 3) return;
        float[] xy = new float[count * 2];
        // drawVertices reads one colour per vertex but checks the array against the position count.
        int[] colors = new int[count * 2];
        for (int i = 0; i < count; i++) {
            xy[i * 2] = f(p[2 + i * 3]);
            xy[i * 2 + 1] = f(p[3 + i * 3]);
            colors[i] = (int) l(p[4 + i * 3]);
        }
        if (type == 1) {
            stroke.setStyle(Paint.Style.STROKE);
            stroke.setShader(null);
            stroke.setStrokeWidth(1f);
            for (int i = 0; i < count; i++) {
                stroke.setColor(colors[i]);
                canvas.drawPoint(xy[i * 2], xy[i * 2 + 1], stroke);
            }
            return;
        }
        if (type == 2 || type == 3) {
            stroke.setStyle(Paint.Style.STROKE);
            stroke.setStrokeWidth(1f);
            int step = type == 2 ? 2 : 1;
            for (int i = 0; i + 1 < count; i += step) {
                stroke.setColor(colors[i]);
                stroke.setShader(null);
                if (colors[i] != colors[i + 1] && (xy[i * 2] != xy[i * 2 + 2] || xy[i * 2 + 1] != xy[i * 2 + 3])) {
                    stroke.setShader(new android.graphics.LinearGradient(
                            xy[i * 2], xy[i * 2 + 1], xy[i * 2 + 2], xy[i * 2 + 3],
                            colors[i], colors[i + 1], android.graphics.Shader.TileMode.CLAMP));
                    stroke.setColor(Color.WHITE);
                }
                canvas.drawLine(xy[i * 2], xy[i * 2 + 1], xy[i * 2 + 2], xy[i * 2 + 3], stroke);
            }
            stroke.setShader(null);
            return;
        }
        if (type < 4 || type > 6 || count < 3) return;
        fill.setColor(Color.WHITE);
        if (android.os.Build.VERSION.SDK_INT >= 29) {
            Canvas.VertexMode mode = type == 4 ? Canvas.VertexMode.TRIANGLES
                    : type == 5 ? Canvas.VertexMode.TRIANGLE_STRIP : Canvas.VertexMode.TRIANGLE_FAN;
            int used = type == 4 ? count - count % 3 : count;
            canvas.drawVertices(mode, used * 2, xy, 0, null, 0, colors, 0, null, 0, 0, fill);
            return;
        }
        int step = type == 4 ? 3 : 1;
        for (int i = 0; i + 2 < count; i += step) {
            int a = type == 6 ? 0 : i;
            int b = i + 1, c = i + 2;
            triangle.rewind();
            triangle.moveTo(xy[a * 2], xy[a * 2 + 1]);
            triangle.lineTo(xy[b * 2], xy[b * 2 + 1]);
            triangle.lineTo(xy[c * 2], xy[c * 2 + 1]);
            triangle.close();
            fill.setColor(colors[a]);
            canvas.drawPath(triangle, fill);
        }
    }

    // "I dx dy dw dh sx sy sw sh color rot ox oy path" from LuaVMStub.
    // sw or sh below zero means the whole PNG (dxDrawImage). Otherwise the
    // source rectangle is in PNG pixels (dxDrawImageSection).
    private void drawImage(Canvas canvas, String body) {
        String[] p = body.split(" ", 13);
        if (p.length < 13 || p[12].isEmpty()) return;
        Bitmap bitmap = bitmapFor(p[12]);
        if (bitmap == null) return;
        float x = f(p[0]), y = f(p[1]), w = f(p[2]), h = f(p[3]);
        float sx = f(p[4]), sy = f(p[5]), sw = f(p[6]), sh = f(p[7]);
        if (sw < 0f || sh < 0f) source.set(0, 0, bitmap.getWidth(), bitmap.getHeight());
        else {
            source.set(Math.round(sx), Math.round(sy), Math.round(sx + sw), Math.round(sy + sh));
            if (!source.intersect(0, 0, bitmap.getWidth(), bitmap.getHeight())) return;
        }
        if (source.isEmpty()) return;
        int color = (int) l(p[8]);
        image.setColorFilter(color == 0xFFFFFFFF
                ? null
                : new PorterDuffColorFilter(color, PorterDuff.Mode.MULTIPLY));
        destination.set(x, y, x + w, y + h);
        float rotation = f(p[9]);
        canvas.save();
        if (rotation != 0f) canvas.rotate(rotation, x + w * 0.5f + f(p[10]), y + h * 0.5f + f(p[11]));
        canvas.drawBitmap(bitmap, source, destination, image);
        canvas.restore();
    }

    private Bitmap bitmapFor(String path) {
        if (path.startsWith("rt:") || path.startsWith("tx:")) {
            synchronized (MEMORY) {
                Bitmap stored = MEMORY.get(path);
                return stored == null || stored.isRecycled() ? null : stored;
            }
        }
        Bitmap cached = images.get(path);
        if (cached != null) return cached.isRecycled() ? null : cached;
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        Bitmap decoded = BitmapFactory.decodeFile(path, options);
        if (decoded != null) images.put(path, decoded);
        return decoded;
    }

    private void drawRect(Canvas canvas, String body) {
        String[] p = body.split(" ");
        if (p.length < 5) return;
        float x = f(p[0]), y = f(p[1]), w = f(p[2]), h = f(p[3]);
        fill.setColor((int) l(p[4]));
        // Desktop draws the quad through ID3DXSprite, which shifts it half a pixel
        // up and left. Rounding every edge left the left and top sides one pixel
        // inside the desktop border; the right edge already landed on the right pixel.
        float left = (float) Math.floor(x - 0.5f);
        float top = (float) Math.floor(y - 0.5f);
        float right = Math.round(x + w);
        float bottom = Math.round(y + h);
        if (right < left + 1f) right = left + 1f;
        if (bottom < top + 1f) bottom = top + 1f;
        canvas.drawRect(left, top, right, bottom, fill);
    }

    private void drawText(Canvas canvas, String body) {
        String[] p = body.split(" ", 8);
        if (p.length < 8) return;
        // Desktop floors the text box when sub-pixel positioning is off
        // (CGraphics::DrawStringQueued). Extra font padding sat the glyphs inside
        // the left and top of that box.
        float x = (float) Math.floor(f(p[0]));
        float y = (float) Math.floor(f(p[1]));
        float right = (float) Math.floor(f(p[2]));
        float bottom = (float) Math.floor(f(p[3]));
        float scale = f(p[4]);
        text.setColor((int) l(p[5]));
        // The draw line is in the pixels that guiGetScreenSize reports (no density factor).
        // Desktop draws the default font at 15 px times the scale (CGraphics fontInfos).
        text.setTextSize(textBase * scale);
        CharSequence shown = "1".equals(p[6]) ? MtaChatView.colorize(p[7]) : p[7];
        int width = Math.max(1, (int) (right - x));
        StaticLayout layout = StaticLayout.Builder.obtain(shown, 0, shown.length(), text, width)
                .setIncludePad(false).setMaxLines(3).build();
        canvas.save();
        canvas.translate(x, y + Math.max(0, (bottom - y - layout.getHeight()) / 2f));
        layout.draw(canvas);
        canvas.restore();
    }

    private static float f(String value) {
        try { return Float.parseFloat(value); } catch (NumberFormatException error) { return 0f; }
    }

    private static long l(String value) {
        try { return Long.parseLong(value); } catch (NumberFormatException error) { return Color.WHITE; }
    }
}
