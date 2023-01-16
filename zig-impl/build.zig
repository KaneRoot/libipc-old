const std = @import("std");

const build_dir = "build";

pub fn build(b: *std.build.Builder) void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    // const mode = b.standardReleaseOptions();
    const mode = std.builtin.Mode.Debug;
    // const mode = std.builtin.Mode.ReleaseSmall;

    const lib = b.addStaticLibrary("ipc", "src/bindings.zig");
    lib.setOutputDir(build_dir);
    lib.linkLibC();
    lib.setBuildMode(mode);
    lib.install();

    const solib = b.addSharedLibrary("ipc", "src/bindings.zig", b.version(0, 1, 0));
    solib.setOutputDir(build_dir);
    solib.linkLibC();
    solib.setBuildMode(mode);
    solib.install();

    const main_tests = b.addTest("src/main.zig");
    main_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&main_tests.step);
}
