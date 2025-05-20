package src.util;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class FileUtils {
    public static List<String> findVerilogFiles(String directory) {
        List<String> verilogFiles = new ArrayList<>();
        File dir = new File(directory);
        
        if (dir.exists() && dir.isDirectory()) {
            File[] files = dir.listFiles();
            if (files != null) {
                for (File file : files) {
                    if (file.isFile() && file.getName().endsWith(".v")) {
                        verilogFiles.add(file.getAbsolutePath());
                    }
                }
            }
        }
        
        return verilogFiles;
    }
}
