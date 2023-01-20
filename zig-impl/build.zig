const std = @import("std");

const bin_dir   = "bin";
const build_dir = "build";

pub fn build(b: *std.build.Builder) void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    // const mode = b.standardReleaseOptions();
    // const mode = std.builtin.Mode.Debug;
    const mode = std.builtin.Mode.ReleaseSmall;

    const ipcd_exe = b.addExecutable("ipcd", "apps/ipcd.zig");
    ipcd_exe.addPackagePath("ipc", "./src/ipc.zig");
    ipcd_exe.setOutputDir(bin_dir);
    ipcd_exe.linkLibC();
    ipcd_exe.setBuildMode(mode);
    ipcd_exe.install();

    const tcpd_exe = b.addExecutable("tcpd", "apps/tcpd.zig");
    tcpd_exe.addPackagePath("ipc", "./src/ipc.zig");
    tcpd_exe.setOutputDir(bin_dir);
    tcpd_exe.linkLibC();
    tcpd_exe.setBuildMode(mode);
    tcpd_exe.install();

    const pong_exe = b.addExecutable("pong", "apps/pong.zig");
    pong_exe.addPackagePath("ipc", "./src/ipc.zig");
    pong_exe.setOutputDir(bin_dir);
    pong_exe.linkLibC();
    pong_exe.setBuildMode(mode);
    pong_exe.install();


    const pongd_exe = b.addExecutable("pongd", "apps/pongd.zig");
    pongd_exe.addPackagePath("ipc", "./src/ipc.zig");
    pongd_exe.setOutputDir(bin_dir);
    pongd_exe.linkLibC();
    pongd_exe.setBuildMode(mode);
    pongd_exe.install();

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
