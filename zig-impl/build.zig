const std = @import("std");

const VERSION = "0.1.0";

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    const is_release = b.option(bool, "release", "Compile a release build.") orelse false;

    if (is_release) {
        std.log.err("hello, this is the release stuff", .{});
    }
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const static_lib = b.addStaticLibrary(.{
        .name = "ipc",
        // In this case the main source file is merely a path, however, in more
        // complicated build scripts, this could be a generated file.
        .root_source_file = .{ .path = "src/bindings.zig" },
        .target = target,
        .optimize = optimize,
    });

    // Link with the libc of the target system since the C allocator
    // is required in the bindings.
    static_lib.linkLibC();

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    static_lib.install();

    const shared_lib = b.addSharedLibrary(.{
        .name = "ipc",
        .root_source_file = .{ .path = "src/bindings.zig" },
        .version = comptime (try std.builtin.Version.parse(VERSION)),
        .target = target,
        .optimize = optimize,
    });
    shared_lib.linkLibC();
    shared_lib.install();

    // Creates a step for unit testing.
    const main_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    main_tests.linkLibC();

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build test`
    // This will evaluate the `test` step rather than the default, which is "install".
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&main_tests.step);

    const install_static_lib = b.addInstallArtifact(static_lib);
    const static_lib_step = b.step("static", "Compile LibIPC as a static library.");
    static_lib_step.dependOn(&install_static_lib.step);

    const install_shared_lib = b.addInstallArtifact(shared_lib);
    // b.getInstallStep().dependOn(&install_shared_lib.step);
    const shared_lib_step = b.step("shared", "Compile LibIPC as a shared library.");
    shared_lib_step.dependOn(&install_shared_lib.step);
}
