package android.text;
public class StaticLayout extends Layout {
    private final int lines;
    private final float extra;
    private StaticLayout(CharSequence value,float extra) { lines=value.toString().split("\n",-1).length; this.extra=extra; }
    public int getHeight() { return (int)(lines*15+(lines-1)*extra); }
    public void draw(android.graphics.Canvas canvas) { canvas.recordText(); }
    public static class Builder {
        private CharSequence value;
        private float extra;
        public static Builder obtain(CharSequence text,int start,int end,TextPaint paint,int width) { Builder b=new Builder(); b.value=text.subSequence(start,end); return b; }
        public Builder setIncludePad(boolean value) { return this; }
        public Builder setAlignment(Alignment value) { return this; }
        public Builder setLineSpacing(float extra,float multiple) { this.extra=extra; return this; }
        public StaticLayout build() { return new StaticLayout(value,extra); }
    }
}
