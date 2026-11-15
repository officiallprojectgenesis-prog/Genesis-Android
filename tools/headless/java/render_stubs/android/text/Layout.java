package android.text;
public class Layout {
    public enum Alignment { ALIGN_NORMAL, ALIGN_CENTER, ALIGN_OPPOSITE }
    public static float getDesiredWidth(CharSequence value,TextPaint paint) {
        int width=0;
        for(String line:value.toString().split("\n",-1)) width=Math.max(width,line.length()*8);
        return width;
    }
}
