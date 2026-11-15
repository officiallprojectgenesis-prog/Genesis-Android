package com.worldrpg.neonlauncher.game;

import java.util.LinkedHashMap;
import java.util.Map;

/** Bounded cache keyed by native font instance, path and weight. */
final class DxFontCache<T> {
    interface Loader<T> { T load(String path, boolean bold); }
    private final int capacity;
    private final Loader<T> loader;
    private final LinkedHashMap<String, T> entries = new LinkedHashMap<>(16, 0.75f, true);

    DxFontCache(int capacity, Loader<T> loader) {
        if (capacity < 1) throw new IllegalArgumentException("font cache capacity");
        this.capacity = capacity;
        this.loader = loader;
    }

    synchronized T get(String source, boolean bold) {
        String key = (bold ? "b:" : "n:") + source;
        T value = entries.get(key);
        if (value != null) return value;
        value = loader.load(path(source), bold);
        entries.put(key, value);
        if (entries.size() > capacity) {
            Map.Entry<String, T> oldest = entries.entrySet().iterator().next();
            entries.remove(oldest.getKey());
        }
        return value;
    }

    static String path(String source) {
        if (source.startsWith("@font:")) {
            int end = source.indexOf(':', 6);
            if (end >= 0) return source.substring(end + 1);
        }
        return source;
    }
}
