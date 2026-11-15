package android.graphics;
public class Canvas {
    public static final java.util.List<String> draws = new java.util.ArrayList<>();
    public final java.util.List<Float> textY = new java.util.ArrayList<>();
    private final java.util.ArrayDeque<Float> saved = new java.util.ArrayDeque<>();
    private boolean target;
    private float y;
    public int pixel;
    public Canvas() {}
    public Canvas(Bitmap bitmap) { target=true; }
    public int save() { saved.push(y); return saved.size(); }
    public void restore() { y=saved.pop(); }
    public void translate(float x,float deltaY) { y+=deltaY; }
    public void scale(float x,float y) {}
    public void rotate(float angle,float x,float y) {}
    public boolean clipRect(float l,float t,float r,float b) { return true; }
    public void recordText() { textY.add(y); }
    public void drawRect(float l,float t,float r,float b,Paint paint) {
        PorterDuff.Mode mode=paint.xfer instanceof PorterDuffXfermode ? ((PorterDuffXfermode)paint.xfer).mode : PorterDuff.Mode.SRC_OVER;
        draws.add((target?"RT:":"screen:")+mode);
        int a=paint.color>>>24;
        if (mode==PorterDuff.Mode.SRC || a==255 && mode!=PorterDuff.Mode.ADD) pixel=paint.color;
        else if (mode==PorterDuff.Mode.ADD) pixel=0xff000000 | Math.min(255,((pixel>>16)&255)+((paint.color>>16)&255))<<16 | Math.min(255,((pixel>>8)&255)+((paint.color>>8)&255))<<8 | Math.min(255,(pixel&255)+(paint.color&255));
    }
}
