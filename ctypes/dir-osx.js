var coreFoundation = ctypes.open("/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation");

const _Dir = new ctypes.StructType("Dir").ptr;

const _dirent = new ctypes.StructType("Directory entry", [
    {"d_ino": ctypes.uint32_t},
    {"d_reclen": ctypes.uint16_t},
    {"d_type": ctypes.uint8_t},
    {"d_namelen": ctypes.uint8_t},
    {"d_name": ctypes.char.array(256)}
]);

var opendir = coreFoundation.declare("opendir", ctypes.default_abi, _Dir, ctypes.char.ptr);
var closedir = coreFoundation.declare("closedir", ctypes.default_abi, ctypes.int, _Dir);
var readdir = coreFoundation.declare("readdir", ctypes.default_abi, _dirent.ptr, _Dir);

var ret = opendir("/Users/anthonycatel/");
if (!ret.isNull()) {
    while(1) {
        var val = readdir(ret);
        if (val.isNull()) break;

        echo(val.contents.d_name.readString());
    }

    closedir(ret);
}
