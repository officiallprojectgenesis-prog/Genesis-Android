package com.worldrpg.neonlauncher.game;

import android.graphics.Canvas;
import android.graphics.Typeface;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Map;

/** Executes the real MtaDxView methods against small Android test doubles. */
public final class DxViewRegressionTest {
    private static int checks;
    private static void check(boolean value) {
        checks++;
        if (!value) throw new AssertionError("DX view check " + checks);
    }
    public static void main(String[] args) throws Exception {
        MtaDxView view = new MtaDxView(null);
        view.setFrame("B 3\nG 7 16 16 1 0\nR 0 0 8 8 4294967295\nG\nB 0\n");
        view.setFrame("R 0 0 1 1 4294967295\n");
        view.onDraw(new Canvas());
        check(Canvas.draws.get(0).equals("RT:SRC"));
        check(Canvas.draws.get(1).equals("screen:SRC_OVER"));

        Field field = MtaDxView.class.getDeclaredField("MEMORY");
        field.setAccessible(true);
        Map<?,?> memory = (Map<?,?>)field.get(null);
        view.setFrame("G 8 16 16 1 0\nR 0 0 8 8 4294967295\nG\n");
        view.setFrame("D 8\n");
        view.onDraw(new Canvas());
        check(!memory.containsKey("rt:8"));
        view.setFrame("G 9 16 16 1 0\nR 0 0 8 8 4294967295\nG\n");
        view.setFrame("D 9\n");
        view.setFrame("");
        check(view.getVisibility() == android.view.View.VISIBLE);
        view.onDraw(new Canvas());
        check(!memory.containsKey("rt:9"));

        Canvas color = new Canvas();
        view.setFrame("B 0\nR 0 0 8 8 4278190335\nB 1\nR 0 0 8 8 4294901760\n");
        view.onDraw(color);
        check(color.pixel == 0xffff0000); // Opaque red replaces blue; saturated ADD would yield magenta.

        Path font = Files.createTempFile("dx-view-font", ".ttf");
        Method lookup = MtaDxView.class.getDeclaredMethod("typefaceFor",String.class,boolean.class);
        lookup.setAccessible(true);
        try {
            Files.write(font,"version-one".getBytes(StandardCharsets.UTF_8));
            Typeface old = (Typeface)lookup.invoke(null,"@font:1:" + font,false);
            Files.write(font,"version-two".getBytes(StandardCharsets.UTF_8));
            Typeface fresh = (Typeface)lookup.invoke(null,"@font:2:" + font,false);
            check(old.version.equals("version-one"));
            check(fresh.version.equals("version-two"));
            check(old != fresh);
        } finally { Files.deleteIfExists(font); }

        Canvas bottom = new Canvas();
        view.setFrame("X 0 0 100 300 1 1 4294967295 1 0 2 0 0 0 0 0 0 100 x\\ny\n");
        view.onDraw(bottom);
        check(bottom.textY.size() == 2);
        check(bottom.textY.get(0) == 100f && bottom.textY.get(1) == 200f);
        Canvas center = new Canvas();
        view.setFrame("X 0 0 100 300 1 1 4294967295 1 0 1 0 0 0 0 0 0 100 x\n");
        view.onDraw(center);
        check(center.textY.size() == 1 && center.textY.get(0) == 100f);
        System.out.println("DxViewRegressionTest: " + checks + " checks, 0 failures");
    }
}
