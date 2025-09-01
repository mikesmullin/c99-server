- you are helping me form a set of code style guidelines for my AI Agent (Prompt Engineering)
- help me describe my style of C99 code, such that an AI Agent could observe my coding conventions
- read my `docs/**/*.md` which describes many of these conventions and qualities
- append your complete/comprehensive version (list of observations from my code) to the bottom of this file

- things like:
  - all simple C99
  - not using any of the std lib
  - cross-platform / portable code
  - use of Arena memory management
  - unity build (single compilation unit)
  - data-oriented design (DoD) w/ SoA + SIMD
  - hot-reload via .dll
  - the naming convention of my functions (which resembles OOP classes, but remains strictly C99 FP)
  - compiling with clang, use of clangd to format code (see: `.clang-format`)
  - building with node.js script (see: `build_scripts/Makefile.mjs`)
  - using VSCode as my IDE
  - leveraging heavy use of AI Agents like Claude Sonnet 4 (an AI-first code repo)
    - extremely limited use of vendor/third-party code generally
      - preferring to use AI to generate minimalist implementations of whatever dependencies are needed
        - thereby ensuring the follow all my coding conventions and are compatible with my overall system and deploy targets (platform)

  - **Fixed Upper Bound for Loops**:
    - Recommended for All loops to have a hard upper limit defined as an integer
    - Prevents edge cases that could cause runaway code during iterations, such as linked list traversals.

  - **Functions Perform One Task**:
    - Each function should execute a single, well-defined action, even if it requires multiple steps.
    - Recommended to limit functions to 60 lines or less (about the size of a piece of paper) for readability and testability.
    - Ensures functions are concise, testable as single units, and easy to audit.

  - **Data Hiding**:
    - Declare variables at the lowest scope possible to minimize access and reduce potential errors.
    - Enhances code safety and simplifies debugging by limiting where variables can be modified.

  - **Check All Return Values**:
    - Verify return values for all non-void functions to catch potential errors.
      - prefer to return an integer (signaling success/fail) and instead return any other data by mutating function parameters.
    - Explicitly cast ignored return values to `void` to indicate intentional omission during code reviews.
    - Prevents oversight of critical function behavior, even for seemingly reliable functions like `printf`.

  - **Limit C Preprocessor Use**:
    - Restrict preprocessor to file inclusions and simple conditional macros.
      - mainly for generating structs, or log output
    - Avoid complex macros and conditional compilation (e.g., multiple compile-time flags), as they obscure code clarity and exponentially increase testing requirements.

  - **Restrict Function Pointer Use**:
    - Avoid function pointers, as they complicate the control flow graph and hinder static analysis and testing.
      - mainly because of hot-reloading
      - exception: will allow it if function pointers are defined within static array (this allows lookup of fp like a handle, using array offset, and these won't change after compile time)

  - **Compile with Strict Settings**:
    - Enable all compiler warnings and use pedantic mode to treat warnings as errors.
    - Ensures all potential issues are addressed during compilation.

  - **Use Multiple Static Analyzers and Unit Testing**:
    - Analyze code with multiple static code analyzers using different rule sets to catch diverse issues.
    - Implement thorough unit testing to validate code functionality before deployment.

  These rules prioritize safety, simplicity, and testability, making code easier to statically analyze and robust.

---

## Complete AI Agent Code Style Guidelines for C99

Based on comprehensive analysis of your documentation and codebase, here are the complete coding conventions for this AI-first, high-performance game engine:

### **Project Architecture Principles**

- **Unity Build System**: All C files included into single compilation units (`main.exe` and `game.dll`)
- **Hot-Reload Architecture**: Minimal runtime (`main.exe`) + hot-reloadable game logic (`game.dll`)
- **Cross-Platform Targeting**: Windows/Linux/Web with identical behavior
- **AI-First Development**: Extreme minimal vendor dependencies, prefer AI-generated implementations following these conventions
- **Data-Oriented Design**: Favor Structure-of-Arrays over Array-of-Structures for cache efficiency

### **Language & Standards**

- **Strict C99**: No C++ features, no C11/C23 extensions
- **No Standard Library**: Avoid `<stdlib.h>`, implement custom alternatives (except platform-specific needs)
- **No malloc()**: Use arena allocators exclusively for predictable memory management
- **Fixed-Width Types**: Always use `u8`, `u16`, `u32`, `u64`, `s8`, `s16`, `s32`, `s64`, `f32`, `f64` from `<stdint.h>`, and `bool` from `<stdbool.h>`

### **Naming Conventions**

**Module Functions** (Primary Pattern):
```c
// Long Pattern: ModuleName__functionName
Arena__Push(arena, size);
Math__randomf(min, max, seed);
ECS__field(entity, component);

// Short Pattern: ABBREV_functionName (for most frequently used functions)
// ABBREV can be ALL_CAPS
SZ_overflowRead(); // SZ is the short name for ByteBuffer class
// or can be named after a type like v3 (Vector3)
v3_lerp(t, a, b);
```

**Types**:
```c
// Structs: PascalCase
typedef struct {
  f32 x, y, z;
} CmpTransform3D;

// Enums: ALL_CAPS with prefix
typedef enum {
  CTRANSFORM3D,
  CRIGIDBODY3D,
  CSTATS
} CmpKind;

// Type aliases: snake_case
typedef u64 GID;
typedef struct Arena Arena;
```

**Variables**: 
- `snake_case` for local variables and struct members
- `_camelCase` with underscore prefix for globals
- Single letter for common math: `v` (vector), `m` (matrix), `t` (time), `f` (float)
- any reference to `count` should be abbreviated `ct`
- any reference to `size` should be abbreviated `sz`
- any reference to `length` should be abbreviated `len`

**Constants**:
```c
#define MAX_ENTITIES (2000)           // Screaming snake case
#define Math__PI32 (3.14159f)         // Module-scoped constants
static const f32 GRAVITY = 9.81f;     // File-scoped constants
```

**File Organization**:
```c
// Header guard always #pragma once
#pragma once
#include "../unity.h"  // IWYU pragma: keep

// Forward declarations first
typedef struct Arena Arena;

// Module functions grouped together
void Arena__Push(Arena* arena, u64 size);
void Arena__Reset(Arena* arena);
inline u32 Arena__used(Arena* arena) {
  return arena->pos - arena->buf;
}
```

### **Function Design Patterns**

**Return Value Strategy**:
```c
// Prefer returning status/error codes, output via parameters
bool Math__intersect(v3 ray_origin, v3 ray_dir, v3* hit_point);

// Simple getters can return directly
inline f32 v3_mag(v3 a) {
  return Math__sqrtf(v3_dot(a, a));
}

// Always check return values (cast to void if intentionally ignored)
if (!File__read(path, &buffer)) {
  LOG_ERRORF("Failed to read file: %s", path);
  return false;
}
(void)printf("Debug info\n");  // Intentionally ignored
```

**Function Size & Responsibility**:
- Maximum 60 lines per function (size of paper for readability)
- Single responsibility - one clear action per function  
- Use static helper functions to break down complex operations

**Parameter Patterns**:
```c
// Input parameters first, output parameters last
void Transform__rotate(CmpTransform3D* tf, f32 angle, v3 axis, v3* out_position);

// Const-correct inputs
f32 Math__distance(const v3* a, const v3* b);

// Size parameters immediately after array parameters  
void Math__distribute(u8 total, u8 num_stacks, u8 max_per_stack, Seed* seed, u8* dst);
```

### **Memory Management Patterns**

**Arena Allocation**:
```c
// Global arenas with clear lifetimes
Engine__State {
  Arena* arena;       // Persistent - lasts entire session
  Arena* frameArena;  // Frame-scoped - reset every frame
};

// Allocation from appropriate arena based on lifetime
String8* persistent_data = Arena__Push(_G->arena, size);
char* temp_buffer = Arena__Push(_G->frameArena, 256);

// Sub-arenas for complex operations
Arena* scratch = Arena__SubAlloc(_G->frameArena, 1024);
// ... use scratch arena ...
// Automatically cleaned up when frameArena resets
```

**Memory Safety**:
```c
// Always bounds-check arena allocation
void* Arena__Push(Arena* a, u64 sz) {
  ASSERT_CONTEXT(
    a->pos + sz < a->end,
    "Arena exhausted. requested %llu, available %llu", 
    sz, Arena__remain(a));
  // ... allocation logic
}

// Zero-initialization when needed
void* Arena__PushZero(Arena* a, u64 sz) {
  u8* p = Arena__Push(a, sz);
  memset(p, 0, sz);
  return p;
}
```

### **ECS Architecture Patterns**

**Component Design**:
```c
// Components are pure data, no methods
typedef struct {
  f32 x, y, z;           // Position
  f32 sx, sy, sz;        // Scale  
  f32 rx, ry, rz;        // Rotation
} CmpTransform3D;

// Favor many small components over few large ones
typedef struct {
  f32 mass;
  f32 vx, vy, vz;        // Velocity
  f32 restitution;
} CmpRigidbody3D;
```

**System Implementation**:
```c
// Systems process entities with specific component combinations
void Physics__updateSystem(void) {
  ArchIt it = {0};
  ECS__archq(&it, (CmpKind[]){CTRANSFORM3D, CRIGIDBODY3D}, 2);
  
  while (ECS__query(&it)) {
    CmpTransform3D* tf = (CmpTransform3D*)it.cpg[0];
    CmpRigidbody3D* rb = (CmpRigidbody3D*)it.cpg[1];
    
    // Apply physics to this entity
    tf->x += rb->vx * _G->deltaTime;
    tf->y += rb->vy * _G->deltaTime;
    tf->z += rb->vz * _G->deltaTime;
  }
}
```

**Entity Creation via Prefabs**:
```c
// Data-driven entity creation
GID player = Entity__prefab(EPLAYER, start_x, start_z);
GID enemy = Entity__prefab(ERABBIT, spawn_x, spawn_z);

// Prefabs defined in data tables, not code
[EPLAYER] = {
  &(CmpItem){.billboard=1, .sprite={.def="simon"}},
  &(CmpTransform3D){0},
  &(CmpRigidbody3D){.mass=80.0f},
  &(CmpCollider){.radius=0.25f},
  // ... more components
}
```

### **Math & Vector Conventions**

**Vector Operations**:
```c
// Consistent naming: v{N}_{operation}[S]
v3 position = v3_add(pos_a, pos_b);         // Vector + vector
v3 scaled = v3_mulS(velocity, deltaTime);   // Vector * scalar (S suffix)
f32 distance = v3_dist(target, current);    // Distance between points
v3 normalized = v3_norm(direction);         // Unit vector

// Matrix operations follow similar pattern
m4 transform = m4_mul(projection, view);
v4 result = m4_mul_v4(transform, position);
```

**Math Constants**:
```c
#define Math__PI32 (3.14159265f)
#define Math__TWOPI32 (6.28318531f)
#define Math__HALFPI32 (1.57079633f)
#define Math__DEG2RAD32 (0.01745329f)
#define Math__RAD2DEG32 (57.2957795f)
```

### **Loop Safety & Bounds**

**Fixed Upper Bounds**:
```c
// Always define hard limits to prevent runaway loops
#define MAX_ENTITIES (2000)
#define MAX_PARTICLES (1000)  
#define MAX_COMPONENTS_PER_ENTITY (16)

for (u32 i = 0; i < entity_count && i < MAX_ENTITIES; i++) {
  ProcessEntity(entities[i]);
}

// Use ring buffers for streaming data
#define MAX_AUDIO_VOICES (16)
Voice voices[MAX_AUDIO_VOICES];
RingBuf voice_queue;
```

### **Error Handling & Debugging**

**Assertion Strategy**:
```c
// Debug-only assertions for development
#ifdef DEBUG_SLOW
  ASSERT(arena->buf <= arena->pos);
  ASSERT(entity_count <= MAX_ENTITIES);
#endif

// Context-aware assertions with detailed info
ASSERT_CONTEXT(
  file_size > 0,
  "File empty or read failed: %s, size: %llu",
  filename, file_size);
```

**Logging Patterns**:
```c
// Structured logging with context
LOG_DEBUGF("Entity %llu spawned at (%.2f, %.2f)", entity_id, x, z);
LOG_WARNF("Arena usage high: %s/%s (%.1f%%)", 
  db_used(arena, true), db_cap(arena, true), usage_percent);
LOG_ERRORF("Failed to load asset: %s", asset_path);
```

### **Hot-Reload Compatibility**

**State Preservation**:
```c
// Mark data that survives hot-reload
Engine__State {
  Arena* arena;          // ✓ Preserved
  World world;           // ✓ Preserved (ECS state)
  NetMgr* net;          // ✓ Preserved (connections)
  
  Scene* current_scene;  // ✗ Rebuilt on reload
  Arena* frameArena;     // ✗ Reset on reload
};

// Reload callbacks for state reconstruction
DLL_EXPORT void onreload(Engine__State* state) {
  // Reinitialize systems that don't preserve state
  Scene__rebuild(state->current_level);
  Audio__reconnect_voices(state);
}
```

**Function Pointer Restrictions**:
```c
// Avoid function pointers (they break hot-reload)
// Exception: static arrays that act as lookup tables
typedef void (*SystemUpdateFn)(void);
static SystemUpdateFn SYSTEM_UPDATES[] = {
  Physics__updateSystem,
  AI__updateSystem,
  Render__updateSystem,
  NULL  // Sentinel
};
```

### **Build System Integration**

**Conditional Compilation**:
```c
// Platform detection
#ifdef _WIN32
  // Windows-specific code
#elif __linux__
  // Linux-specific code  
#elif __EMSCRIPTEN__
  // Web-specific code
#endif

// Build target detection
#ifdef ENGINE_MAIN
  // Only in main.exe runtime
  #define SOKOL_IMPL
#endif

#ifdef ENGINE_DLL
  // Only in hot-reloadable game.dll
  #include "game/systems/GameLogic.c"
#endif
```

**Unity Build Organization**:
```c
// Include order matters - dependencies first
#include "game/common/Math.c"         // No dependencies
#include "game/common/Arena.c"        // Depends on Math
#include "game/common/ECS.c"          // Depends on Arena
#include "game/systems/Physics.c"     // Depends on ECS
```

### **Performance Optimization Patterns**

**Data-Oriented Layout**:
```c
// Structure of Arrays for cache efficiency
typedef struct {
  f32* positions_x;  // [MAX_ENTITIES]
  f32* positions_y;  // [MAX_ENTITIES]  
  f32* positions_z;  // [MAX_ENTITIES]
  f32* velocities_x; // [MAX_ENTITIES]
  f32* velocities_y; // [MAX_ENTITIES]
  f32* velocities_z; // [MAX_ENTITIES]
} PhysicsData;

// Process arrays in batch for SIMD potential
for (u32 i = 0; i < entity_count; i++) {
  physics.positions_x[i] += physics.velocities_x[i] * dt;
  physics.positions_y[i] += physics.velocities_y[i] * dt;
  physics.positions_z[i] += physics.velocities_z[i] * dt;
}
```

**Inline Guidelines**:
```c
// Inline small, frequently-called functions
inline f32 Math__min(f32 a, f32 b) {
  return a < b ? a : b;
}

inline bool GI_alive(GID gid) {
  return /* quick validation check */;
}

// Don't inline large functions or those with loops
void ECS__defrag(void);  // Complex function, not inlined
```

### **Cross-Platform Determinism**

**Floating Point Consistency**:
```c
// Use lookup tables for trigonometry (same across all platforms)
static f32 SIN_T[FINE_ANGLES];  // Pre-computed sine table

inline f32 Math__sin(f32 radians) {
  return SIN_T[Math__RAD2FINE32(radians) & FINE_ANGLES_MASK];
}

// Custom implementations of math functions for consistency
f32 Math__sqrtf(f32 n);  // Same result on all platforms
f32 Math__expf(f32 x);   // Maclaurin series approximation
```

**Random Number Generation**:
```c
// Deterministic PRNG with multiple seeds for different subsystems
typedef struct {
  Seed gameplay;   // Game logic randomness
  Seed audio;      // Audio system randomness  
  Seed nosync;     // Non-critical randomness
} SeedBank;

// All systems must use appropriate seed for determinism
f32 damage = Math__randomf(min_dmg, max_dmg, &_G->seeds.gameplay);
```

### **Testing & Validation**

**Unit Test Structure**:
```c
// Tests can include specific modules in isolation
#define ENGINE_TEST
#include "src/unity.h"

void test_vector_normalize() {
  v3 input = {3.0f, 4.0f, 0.0f};
  v3 result = v3_norm(input);
  f32 length = v3_mag(result);
  ASSERT_FLOAT_EQ(length, 1.0f, 0.001f);
}
```

**Dependency Injection for Testing**:
```c
// Mock implementations can be injected
#include DEPINJ(MATH_IMPL, "game/common/Math.c")

// In tests: DEPINJ__MATH_IMPL points to mock
// In production: DEPINJ expands to actual file
```

### **AI Agent Guidance Summary**

When generating code for this project:

1. **Always use the `ModuleName__functionName` pattern** for public functions
2. **Prefer small, single-purpose functions** (≤60 lines)
3. **Use arena allocation exclusively** - no malloc/free
4. **Follow ECS patterns** - pure data components, behavior in systems
5. **Include bounds checking** and fixed upper limits on all loops
6. **Use fixed-width types** (`u32`, `f32`) instead of `int`, `float`
7. **Check all return values** or explicitly cast to `void`
8. **Design for hot-reload** - avoid function pointers, preserve critical state
9. **Optimize for cache efficiency** - prefer SoA over AoS layout
10. **Maintain cross-platform determinism** - use custom math implementations

This codebase prioritizes **performance, determinism, and maintainability** through disciplined engineering practices and AI-assisted development workflows.

### **File Documentation Standard**

Each C source file begins with a structured API documentation table that catalogs all functions by their logical class/module groupings. This serves as both quick reference and architectural overview.

**Documentation Format**:
```c
#pragma once
#include "../unity.h"  // IWYU pragma: keep

// @class ModuleName [(ShortName)]
// Function | Purpose
// --- | ---
// Module__functionName(params) | Brief description of what function does
// Module__anotherFunction(params) | Another function description
// Module__utilityHelper(params) | Helper function description

// @class AnotherModule
// Function | Purpose
// --- | ---
// Other__mainFunction(params) | Primary function description
// Other__supportFunction(params) | Supporting function description
```

**Real Examples from Codebase**:

From `Animate.c`:
```c
// @class Tween
// Function | Purpose
// --- | ---
// Tween__init(tw, from, to, duration) | Initialize tween defaults
// Tween__alloc(from, to, duration) | Create and return a tween
// Tween__value(tw, t) | Interpolate tween at progress t
// Tween__append(root, appendage) | Append tween to chain

// @class Anim
// Function | Purpose
// --- | ---
// Anim__resync(a) | Recompute keyframe offsets and totals
// Anim__play(a) | Start or restart animation
// Anim__pause(a) | Pause animation and emit event
// Anim__completed(a) | Check if animation completed
```

From `ECS.c`:
```c
// @class Generational Index Array (GenIdx)
// Function | Purpose
// --- | ---
// GI_id(egid) | Pack EGID into GID and update GenIdx
// GI_alive(gid) | Check if GID is alive
// GI_kill(gid) | Mark entity dead and recycle slot

// @class ECS
// Function | Purpose
// --- | ---
// ECS__archq(it) | Cache matching archetypes for query
// ECS__query(it) | Iterate entities matching components
// ECS__field(eid, cmp) | Get pointer to component data
```

From `Timer.c`:
```c
// @class Timer
// Function | Purpose
// --- | ---
// T_ended(t, duration) | Check if timer ended
// T_play(t) | Start or restart timer
// T_pct(t, duration) | Get elapsed progress percentage

// @class Cooldown
// Function | Purpose
// --- | ---
// CD_rdy(cd) | Check if cooldown ready or ended
// CD_remain(cd) | Remaining milliseconds
// CD_play(t, duration) | Set cooldown to expire after duration
```

**Documentation Guidelines**:

- **@class Declaration**: Use `@class ModuleName` or `@class ModuleName (ShortPrefix)` for abbreviated function prefixes
- **Table Format**: Always use `Function | Purpose` header with `--- | ---` separator
- **Function Signatures**: Include parameter names to show expected inputs
- **Purpose Descriptions**: Concise, actionable descriptions focusing on what the function accomplishes
  - These descriptions should also appear above each function where it is defined in the file (if no comment is already provided), in the form of a single-line comment (complex functions can have a series of line comments)
- **Logical Grouping**: Group related functions under their conceptual module/class
- **Order by Importance**: List core functions first, utilities and helpers later
- **Consistent Naming**: Function names should follow the `Module__verb` or `PREFIX_verb` pattern


**Benefits of This Documentation Style**:

1. **Quick API Reference**: Developers can instantly see all available functions without reading implementation
2. **Architectural Clarity**: Shows how functions are logically grouped into cohesive modules
3. **Interface Contract**: Documents expected parameters and behavior before implementation details
4. **AI Agent Guidance**: Provides clear structure for AI tools to understand module boundaries and responsibilities
5. **Code Review Aid**: Reviewers can verify implementation matches documented intent
6. **Maintenance Efficiency**: Easy to spot missing functions or identify refactoring opportunities

This documentation pattern reinforces the modular, class-like organization of C99 code while maintaining the functional programming approach throughout the implementation.

### **Unit Testing Architecture**

The codebase implements a comprehensive unit testing system that integrates with the unity build architecture and hot-reload capabilities. Tests are designed for isolated module verification and can be executed individually or in batches.

**Test File Structure**:
```c
#define ENGINE_TEST
#include "../../../../src/unity.h"  // IWYU pragma: keep

// @describe ModuleName
// @tag category
int main() {
  // Test scenarios organized by functionality
  
  // ---
  // Scenario: Feature Description
  {
    // Setup
    Type* instance = Module__create();
    
    // Execute & Verify
    ASSERT(expected_value == Module__getValue(instance));
    ASSERT_CONTEXT(
      condition, 
      "Detailed failure message with context: %d", 
      actual_value);
  }
  
  // Additional scenarios...
  
  return 0;
}
```

**Test Metadata Annotations**:
Tests use comment-based metadata to control execution and organization:

```c
// @describe Math          // Human-readable test description
// @tag common            // Category for filtering (common, net, ux, etc.)
// @skip                  // Skip this test during execution
// @noinject              // Disable dependency injection
// @inject SYMBOL path    // Override specific dependency 
// @run args              // Custom command line arguments
// @stagger 500           // Delay between multi-process tests (ms)
```

**Assertion Patterns**:
```c
// Basic assertions
ASSERT(condition);                    // Simple boolean check
ASSERT(expected == actual);           // Equality check

// Context-aware assertions with detailed error messages
ASSERT_CONTEXT(
  file_size > 0,
  "File read failed: %s, expected size > 0, got %llu",
  filename, file_size);

// Floating point comparisons
ASSERT(Math__aeq(expected, actual, Math__EPSILON));

// Approximate equality for cross-platform determinism
ASSERT(APPROXEQF(0.0f, Math__sin(Math__PI32)));
```

**Test Organization Patterns**:

1. **Scenario-Based Structure**: Tests group related functionality under descriptive scenario comments:
   ```c
   // ---
   // Scenario: Ring Buffer Operations
   {
     RingBuf rb = {0};
     // Test empty buffer
     ASSERT(Ring__empty(&rb));
     // Test push operations
     // Test full buffer handling
   }
   
   // ---
   // Scenario: Entity Component System
   {
     // Test entity creation
     // Test component addition
     // Test query iteration
   }
   ```

2. **Isolated Setup**: Each test scenario creates its own isolated environment:
   ```c
   // Arena allocation for test isolation
   _G->arena = Arena__Alloc(50 * 1024 * 1024);  // 50MB test arena
   
   // Initialize ECS for entity tests
   GI_init();  // Reset generational index
   ```

3. **Progressive Complexity**: Tests start with basic operations and build to complex scenarios:
   ```c
   // Basic creation
   GID entity = GI_next(archetype);
   
   // Component addition
   ECS__add(entity, "test_name", CTRANSFORM3D, &transform_data);
   
   // System interaction
   ArchIt it = {0};
   ECS__archq(&it, (CmpKind[]){CTRANSFORM3D}, 1);
   while (ECS__query(&it)) {
     // Verify system behavior
   }
   ```

**CLI Test Execution**:

**Basic Commands**:
```bash
# Run all tests
node build_scripts/Makefile.mjs test

# Run tests with specific tag filter
node build_scripts/Makefile.mjs test --filter=common
node build_scripts/Makefile.mjs test --filter=net
node build_scripts/Makefile.mjs test --filter=ux

# Run specific test file
node build_scripts/Makefile.mjs test --file=test/unit/game/common/Math.c

# Compile tests without running (build verification)
node build_scripts/Makefile.mjs test --norun

# Watch mode - rebuild and run tests on file changes
node build_scripts/Makefile.mjs test --watch src/

# Clean build before testing
node build_scripts/Makefile.mjs test --clean
```

**Advanced Options**:
```bash
# Disable dependency injection (pure unity build)
node build_scripts/Makefile.mjs test --noinject

# Multiple filters (OR logic)
node build_scripts/Makefile.mjs test --filter=common --filter=math

# Specific file with build-only verification
node build_scripts/Makefile.mjs test --file=test/unit/game/common/ECS.c --norun
```

**Test Output Format**:
```
--tests--

1) Math
   Compilation took 245.67 ms
   process succeeded. code: 0

2) ECS  
   process succeeded. code: 0

3) List
   process succeeded. code: 0

Finished in 1247.89 ms, compiled in 1121.34 ms, execed in 126.55 ms.
12 run, 0 failures, 2 skips. (14 found)
```

**Integration vs Unit Tests**:

- **Unit Tests**: Located in `test/unit/**/*.c`, test individual modules in isolation
- **Integration Tests**: Located in `test/integration/**/app.c`, test system interactions with full game context

**Test Compilation Process**:

1. **Dependency Analysis**: Build system scans `#include` statements to determine required translation units
2. **Selective Compilation**: Only compiles necessary source files for each test
3. **Dependency Injection**: Uses `DEPINJ` macros to substitute test implementations:
   ```c
   // In production: includes actual file
   #include DEPINJ(GAME_H, "src/game/game.c")
   
   // In tests: can inject mock implementation
   // -DDEPINJ__GAME_H="test/mocks/game.c"
   ```
4. **Isolated Executables**: Each test compiles to its own executable for isolation

**Testing Best Practices**:

- **Single Responsibility**: Each test file focuses on one module or related functionality
- **Descriptive Scenarios**: Use meaningful scenario names that describe what's being tested  
- **Assertion Context**: Provide detailed error messages for failed assertions
- **Test Isolation**: Each test should be able to run independently
- **Cross-Platform Consistency**: Use deterministic algorithms and avoid platform-specific behavior
- **Performance Validation**: Include timing assertions for performance-critical code
- **Memory Safety**: Verify arena bounds and memory usage patterns
- **Hot-Reload Compatibility**: Test state preservation and reconstruction scenarios

This testing architecture ensures code reliability while supporting the rapid iteration enabled by hot-reloading and the unity build system.