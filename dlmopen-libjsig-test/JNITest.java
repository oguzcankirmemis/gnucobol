public class JNITest {
    private String testAttribute = "Test Attribute";

    static {
        System.loadLibrary("native");
    }

    public static void main(String[] args) {
        JNITest test = null;
        System.out.println("Hello from Java!");

        System.out.println("Testing COBOL threads with dlmopen and libjsig");
        System.out.println("Test custom signal handler from JNI.");
        new JNITest().runCobolThreadsWithCustomSignalHandler();
        System.out.println("JVM signal handlers should still be intact.");
        System.out.println("Test whether JVM can still recover from NullPointerException (SIGSEGV)");
        try {
            System.out.println(test.testAttribute);
        } catch (NullPointerException err) {
            System.out.println("Error successfully catched by JVM!");
        }

        System.out.println("Testing default signal handler from libcob.");
        new JNITest().runCobolThreadsDefault();
        System.out.println("At this point the signal handlers of the JVM are overwritten!");
        System.out.println("Demonstrating it with a NullPointerException.");
        System.out.println("Test whether JVM can still recover from NullPointerException (SIGSEGV)");
        try {
            System.out.println(test.testAttribute);
        } catch (NullPointerException err) {
            System.out.println("Error successfully catched by JVM!");
        }
    }

    private native void runCobolThreadsDefault();

    private native void runCobolThreadsWithCustomSignalHandler();
}