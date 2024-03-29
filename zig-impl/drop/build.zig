const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const lib = b.addStaticLibrary("ipc", "src/main.zig");
    lib.setOutputDir("build");
    lib.linkLibC();
    lib.setBuildMode(mode);
    lib.install();

    const solib = b.addSharedLibrary("ipc", "src/main.zig", b.version(0, 0, 1));
    solib.setOutputDir("build");
    solib.linkLibC();
    solib.setBuildMode(mode);
    solib.install();

    const main_tests = b.addTest("src/main.zig");
    main_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&main_tests.step);
}
