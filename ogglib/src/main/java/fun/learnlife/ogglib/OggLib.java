package fun.learnlife.ogglib;

public class OggLib {
    static {
        System.loadLibrary("ogg-lib");
    }
    /**
     * 编码为ogg返回
     *
     * @param origin
     * @param length
     * @return
     */
    public native byte[] encode(byte[] origin, int length);

    /**
     * @param inPath  输入的wav文件路径.
     * @param outPath 输出到的ogg文件路径
     * @return
     */
    public native String wavToOgg(String inPath, String outPath);
}
