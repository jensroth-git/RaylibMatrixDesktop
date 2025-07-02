#include "asset_loader.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include "raylib.h"

std::string GetAssetPath(const char* filename) {
    // First try to get from app bundle (for app bundle builds)
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle) {
        CFStringRef fileNameStr = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
        CFURLRef fileURL = CFBundleCopyResourceURL(mainBundle, fileNameStr, NULL, NULL);
        CFRelease(fileNameStr);
        
        if (fileURL) {
            char path[1024];
            if (CFURLGetFileSystemRepresentation(fileURL, TRUE, (UInt8*)path, sizeof(path))) {
                CFRelease(fileURL);
                // Check if file actually exists
                if (FileExists(path)) {
                    return std::string(path);
                }
            }
            CFRelease(fileURL);
        }
    }
    
    // Fallback to assets directory next to executable (for development builds)
    std::string assetsPath = std::string("assets/") + filename;
    if (FileExists(assetsPath.c_str())) {
        return assetsPath;
    }
    
    // Last resort - just return the filename
    return std::string(filename);
}

#endif 