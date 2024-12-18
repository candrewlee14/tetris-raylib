const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const raylib_dep = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
        .shared = optimize == .Debug,
    });
    const raylib = raylib_dep.artifact("raylib");
    b.installArtifact(raylib);

    const exe = b.addExecutable(.{
        .name = "tetris-raylib",
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
    });
    exe.linkLibC();
    exe.linkLibrary(raylib);
    exe.addCSourceFile(.{ .file = .{ .src_path = .{ .owner = b, .sub_path = "src/main.c" } }, .flags = &.{
        "-fPIC",
    } });
    b.installArtifact(exe);

    const game_obj_step = b.step("game-reloadable", "Compile reloadable game object");
    const game_obj = if (optimize == .Debug) b.addSharedLibrary(.{
        .name = "game-reloadable",
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
    }) else b.addStaticLibrary(.{
        .name = "game-reloadable",
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
    });
    game_obj.linkLibrary(raylib);
    game_obj.addCSourceFile(.{ .file = .{ .src_path = .{ .owner = b, .sub_path = "src/game.c" } }, .flags = &.{
        "-fPIC",
    } });
    const game_obj_art = b.addInstallArtifact(game_obj, .{});
    game_obj_step.dependOn(&game_obj_art.step);

    b.installArtifact(game_obj);

    if (optimize == .Debug) {
        exe.root_module.addCMacro("DEBUG", "1");
        game_obj.root_module.addCMacro("DEBUG", "1");
    } else {
        exe.linkLibrary(game_obj);
    }
    switch (target.result.os.tag) {
        .linux => {
            exe.root_module.addCMacro("LINUX", "1");
            game_obj.root_module.addCMacro("LINUX", "1");
        },
        .macos => {
            exe.root_module.addCMacro("MACOS", "1");
            game_obj.root_module.addCMacro("MACOS", "1");
        },
        .windows => {
            exe.root_module.addCMacro("WINDOWS", "1");
            game_obj.root_module.addCMacro("WINDOWS", "1");
        },
        else => @panic("Unsupported OS"),
    }

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
