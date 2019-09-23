/**
 * WebGL2 Compute
 */
/**
 * @constructor
 * @extends {WebGL2RenderingContext}
 */
function WebGL2ComputeRenderingContext() {}

/** @const {number} */
WebGL2ComputeRenderingContext.COMPUTE_SHADER;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_UNIFORM_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_TEXTURE_IMAGE_UNITS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_IMAGE_UNIFORMS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_SHARED_MEMORY_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_UNIFORM_COMPONENTS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_ATOMIC_COUNTERS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_WORK_GROUP_INVOCATIONS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_WORK_GROUP_COUNT;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_WORK_GROUP_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.COMPUTE_WORK_GROUP_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.DISPATCH_INDIRECT_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.DISPATCH_INDIRECT_BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.COMPUTE_SHADER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.DRAW_INDIRECT_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.DRAW_INDIRECT_BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_UNIFORM_LOCATIONS;

/** @const {number} */
WebGL2ComputeRenderingContext.FRAMEBUFFER_DEFAULT_WIDTH;

/** @const {number} */
WebGL2ComputeRenderingContext.FRAMEBUFFER_DEFAULT_HEIGHT;

/** @const {number} */
WebGL2ComputeRenderingContext.FRAMEBUFFER_DEFAULT_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_FRAMEBUFFER_WIDTH;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_FRAMEBUFFER_HEIGHT;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_FRAMEBUFFER_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.UNIFORM;

/** @const {number} */
WebGL2ComputeRenderingContext.UNIFORM_BLOCK;

/** @const {number} */
WebGL2ComputeRenderingContext.PROGRAM_INPUT;

/** @const {number} */
WebGL2ComputeRenderingContext.PROGRAM_OUTPUT;

/** @const {number} */
WebGL2ComputeRenderingContext.BUFFER_VARIABLE;

/** @const {number} */
WebGL2ComputeRenderingContext.SHADER_STORAGE_BLOCK;

/** @const {number} */
WebGL2ComputeRenderingContext.ATOMIC_COUNTER_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.TRANSFORM_FEEDBACK_VARYING;

/** @const {number} */
WebGL2ComputeRenderingContext.ACTIVE_RESOURCES;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_NAME_LENGTH;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_NUM_ACTIVE_VARIABLES;

/** @const {number} */
WebGL2ComputeRenderingContext.NAME_LENGTH;

/** @const {number} */
WebGL2ComputeRenderingContext.TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.ARRAY_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.BLOCK_INDEX;

/** @const {number} */
WebGL2ComputeRenderingContext.ARRAY_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.MATRIX_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.IS_ROW_MAJOR;

/** @const {number} */
WebGL2ComputeRenderingContext.ATOMIC_COUNTER_BUFFER_INDEX;

/** @const {number} */
WebGL2ComputeRenderingContext.BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.BUFFER_DATA_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.NUM_ACTIVE_VARIABLES;

/** @const {number} */
WebGL2ComputeRenderingContext.ACTIVE_VARIABLES;

/** @const {number} */
WebGL2ComputeRenderingContext.REFERENCED_BY_VERTEX_SHADER;

/** @const {number} */
WebGL2ComputeRenderingContext.REFERENCED_BY_FRAGMENT_SHADER;

/** @const {number} */
WebGL2ComputeRenderingContext.REFERENCED_BY_COMPUTE_SHADER;

/** @const {number} */
WebGL2ComputeRenderingContext.TOP_LEVEL_ARRAY_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.TOP_LEVEL_ARRAY_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.LOCATION;

/** @const {number} */
WebGL2ComputeRenderingContext.VERTEX_SHADER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.FRAGMENT_SHADER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.ALL_SHADER_BITS;

/** @const {number} */
WebGL2ComputeRenderingContext.ATOMIC_COUNTER_BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.ATOMIC_COUNTER_BUFFER_START;

/** @const {number} */
WebGL2ComputeRenderingContext.ATOMIC_COUNTER_BUFFER_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMBINED_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_VERTEX_ATOMIC_COUNTERS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_FRAGMENT_ATOMIC_COUNTERS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMBINED_ATOMIC_COUNTERS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_ATOMIC_COUNTER_BUFFER_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;

/** @const {number} */
WebGL2ComputeRenderingContext.ACTIVE_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.UNSIGNED_INT_ATOMIC_COUNTER;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_IMAGE_UNITS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_VERTEX_IMAGE_UNIFORMS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_FRAGMENT_IMAGE_UNIFORMS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMBINED_IMAGE_UNIFORMS;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_BINDING_NAME;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_BINDING_LEVEL;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_BINDING_LAYERED;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_BINDING_LAYER;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_BINDING_ACCESS;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_BINDING_FORMAT;

/** @const {number} */
WebGL2ComputeRenderingContext.VERTEX_ATTRIB_ARRAY_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.ELEMENT_ARRAY_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.UNIFORM_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_FETCH_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.SHADER_IMAGE_ACCESS_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.COMMAND_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.PIXEL_BUFFER_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_UPDATE_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.BUFFER_UPDATE_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.FRAMEBUFFER_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.TRANSFORM_FEEDBACK_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.ATOMIC_COUNTER_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.SHADER_STORAGE_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.ALL_BARRIER_BITS;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_2D;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_3D;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_CUBE;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_2D_ARRAY;

/** @const {number} */
WebGL2ComputeRenderingContext.INT_IMAGE_2D;

/** @const {number} */
WebGL2ComputeRenderingContext.INT_IMAGE_3D;

/** @const {number} */
WebGL2ComputeRenderingContext.INT_IMAGE_CUBE;

/** @const {number} */
WebGL2ComputeRenderingContext.INT_IMAGE_2D_ARRAY;

/** @const {number} */
WebGL2ComputeRenderingContext.UNSIGNED_INT_IMAGE_2D;

/** @const {number} */
WebGL2ComputeRenderingContext.UNSIGNED_INT_IMAGE_3D;

/** @const {number} */
WebGL2ComputeRenderingContext.UNSIGNED_INT_IMAGE_CUBE;

/** @const {number} */
WebGL2ComputeRenderingContext.UNSIGNED_INT_IMAGE_2D_ARRAY;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_FORMAT_COMPATIBILITY_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_FORMAT_COMPATIBILITY_BY_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.IMAGE_FORMAT_COMPATIBILITY_BY_CLASS;

/** @const {number} */
WebGL2ComputeRenderingContext.READ_ONLY;

/** @const {number} */
WebGL2ComputeRenderingContext.WRITE_ONLY;

/** @const {number} */
WebGL2ComputeRenderingContext.READ_WRITE;

/** @const {number} */
WebGL2ComputeRenderingContext.SHADER_STORAGE_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.SHADER_STORAGE_BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.SHADER_STORAGE_BUFFER_START;

/** @const {number} */
WebGL2ComputeRenderingContext.SHADER_STORAGE_BUFFER_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_VERTEX_SHADER_STORAGE_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_FRAGMENT_SHADER_STORAGE_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMPUTE_SHADER_STORAGE_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMBINED_SHADER_STORAGE_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_SHADER_STORAGE_BUFFER_BINDINGS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_SHADER_STORAGE_BLOCK_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COMBINED_SHADER_OUTPUT_RESOURCES;

/** @const {number} */
WebGL2ComputeRenderingContext.DEPTH_STENCIL_TEXTURE_MODE;

/** @const {number} */
WebGL2ComputeRenderingContext.STENCIL_INDEX;

/** @const {number} */
WebGL2ComputeRenderingContext.MIN_PROGRAM_TEXTURE_GATHER_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_PROGRAM_TEXTURE_GATHER_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.SAMPLE_POSITION;

/** @const {number} */
WebGL2ComputeRenderingContext.SAMPLE_MASK;

/** @const {number} */
WebGL2ComputeRenderingContext.SAMPLE_MASK_VALUE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_SAMPLE_MASK_WORDS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_COLOR_TEXTURE_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_DEPTH_TEXTURE_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_INTEGER_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_BINDING_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_FIXED_SAMPLE_LOCATIONS;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_WIDTH;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_HEIGHT;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_DEPTH;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_INTERNAL_FORMAT;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_RED_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_GREEN_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_BLUE_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_ALPHA_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_DEPTH_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_STENCIL_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_SHARED_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_RED_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_GREEN_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_BLUE_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_ALPHA_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_DEPTH_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.TEXTURE_COMPRESSED;

/** @const {number} */
WebGL2ComputeRenderingContext.SAMPLER_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.INT_SAMPLER_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.VERTEX_ATTRIB_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.VERTEX_ATTRIB_RELATIVE_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.VERTEX_BINDING_DIVISOR;

/** @const {number} */
WebGL2ComputeRenderingContext.VERTEX_BINDING_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.VERTEX_BINDING_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.VERTEX_BINDING_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_VERTEX_ATTRIB_RELATIVE_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_VERTEX_ATTRIB_BINDINGS;

/** @const {number} */
WebGL2ComputeRenderingContext.MAX_VERTEX_ATTRIB_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.COMPUTE_SHADER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_UNIFORM_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_TEXTURE_IMAGE_UNITS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_IMAGE_UNIFORMS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_SHARED_MEMORY_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_UNIFORM_COMPONENTS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_ATOMIC_COUNTERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_WORK_GROUP_INVOCATIONS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_WORK_GROUP_COUNT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_WORK_GROUP_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.COMPUTE_WORK_GROUP_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.DISPATCH_INDIRECT_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.DISPATCH_INDIRECT_BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.COMPUTE_SHADER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.DRAW_INDIRECT_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.DRAW_INDIRECT_BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_UNIFORM_LOCATIONS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.FRAMEBUFFER_DEFAULT_WIDTH;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.FRAMEBUFFER_DEFAULT_HEIGHT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.FRAMEBUFFER_DEFAULT_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_FRAMEBUFFER_WIDTH;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_FRAMEBUFFER_HEIGHT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_FRAMEBUFFER_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNIFORM;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNIFORM_BLOCK;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.PROGRAM_INPUT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.PROGRAM_OUTPUT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.BUFFER_VARIABLE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SHADER_STORAGE_BLOCK;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ATOMIC_COUNTER_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TRANSFORM_FEEDBACK_VARYING;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ACTIVE_RESOURCES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_NAME_LENGTH;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_NUM_ACTIVE_VARIABLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.NAME_LENGTH;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ARRAY_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.BLOCK_INDEX;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ARRAY_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MATRIX_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IS_ROW_MAJOR;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ATOMIC_COUNTER_BUFFER_INDEX;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.BUFFER_DATA_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.NUM_ACTIVE_VARIABLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ACTIVE_VARIABLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.REFERENCED_BY_VERTEX_SHADER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.REFERENCED_BY_FRAGMENT_SHADER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.REFERENCED_BY_COMPUTE_SHADER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TOP_LEVEL_ARRAY_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TOP_LEVEL_ARRAY_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.LOCATION;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.VERTEX_SHADER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.FRAGMENT_SHADER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ALL_SHADER_BITS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ATOMIC_COUNTER_BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ATOMIC_COUNTER_BUFFER_START;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ATOMIC_COUNTER_BUFFER_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMBINED_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_VERTEX_ATOMIC_COUNTERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_FRAGMENT_ATOMIC_COUNTERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMBINED_ATOMIC_COUNTERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_ATOMIC_COUNTER_BUFFER_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ACTIVE_ATOMIC_COUNTER_BUFFERS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNSIGNED_INT_ATOMIC_COUNTER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_IMAGE_UNITS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_VERTEX_IMAGE_UNIFORMS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_FRAGMENT_IMAGE_UNIFORMS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMBINED_IMAGE_UNIFORMS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_BINDING_NAME;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_BINDING_LEVEL;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_BINDING_LAYERED;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_BINDING_LAYER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_BINDING_ACCESS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_BINDING_FORMAT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.VERTEX_ATTRIB_ARRAY_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ELEMENT_ARRAY_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNIFORM_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_FETCH_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SHADER_IMAGE_ACCESS_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.COMMAND_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.PIXEL_BUFFER_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_UPDATE_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.BUFFER_UPDATE_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.FRAMEBUFFER_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TRANSFORM_FEEDBACK_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ATOMIC_COUNTER_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SHADER_STORAGE_BARRIER_BIT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.ALL_BARRIER_BITS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_2D;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_3D;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_CUBE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_2D_ARRAY;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.INT_IMAGE_2D;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.INT_IMAGE_3D;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.INT_IMAGE_CUBE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.INT_IMAGE_2D_ARRAY;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNSIGNED_INT_IMAGE_2D;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNSIGNED_INT_IMAGE_3D;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNSIGNED_INT_IMAGE_CUBE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNSIGNED_INT_IMAGE_2D_ARRAY;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_FORMAT_COMPATIBILITY_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_FORMAT_COMPATIBILITY_BY_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.IMAGE_FORMAT_COMPATIBILITY_BY_CLASS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.READ_ONLY;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.WRITE_ONLY;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.READ_WRITE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SHADER_STORAGE_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SHADER_STORAGE_BUFFER_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SHADER_STORAGE_BUFFER_START;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SHADER_STORAGE_BUFFER_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_VERTEX_SHADER_STORAGE_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_FRAGMENT_SHADER_STORAGE_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMPUTE_SHADER_STORAGE_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMBINED_SHADER_STORAGE_BLOCKS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_SHADER_STORAGE_BUFFER_BINDINGS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_SHADER_STORAGE_BLOCK_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COMBINED_SHADER_OUTPUT_RESOURCES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.DEPTH_STENCIL_TEXTURE_MODE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.STENCIL_INDEX;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MIN_PROGRAM_TEXTURE_GATHER_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_PROGRAM_TEXTURE_GATHER_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SAMPLE_POSITION;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SAMPLE_MASK;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SAMPLE_MASK_VALUE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_SAMPLE_MASK_WORDS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_COLOR_TEXTURE_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_DEPTH_TEXTURE_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_INTEGER_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_BINDING_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_SAMPLES;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_FIXED_SAMPLE_LOCATIONS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_WIDTH;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_HEIGHT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_DEPTH;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_INTERNAL_FORMAT;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_RED_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_GREEN_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_BLUE_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_ALPHA_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_DEPTH_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_STENCIL_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_SHARED_SIZE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_RED_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_GREEN_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_BLUE_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_ALPHA_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_DEPTH_TYPE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.TEXTURE_COMPRESSED;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.SAMPLER_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.INT_SAMPLER_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.VERTEX_ATTRIB_BINDING;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.VERTEX_ATTRIB_RELATIVE_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.VERTEX_BINDING_DIVISOR;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.VERTEX_BINDING_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.VERTEX_BINDING_STRIDE;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.VERTEX_BINDING_BUFFER;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_VERTEX_ATTRIB_RELATIVE_OFFSET;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_VERTEX_ATTRIB_BINDINGS;

/** @const {number} */
WebGL2ComputeRenderingContext.prototype.MAX_VERTEX_ATTRIB_STRIDE;

/**
 * @param {number} num_groups_x
 * @param {number} num_groups_y
 * @param {number} num_groups_z
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.dispatchCompute = function(num_groups_x, num_groups_y, num_groups_z) {};

/**
 * @param {number} offset
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.dispatchComputeIndirect = function(offset) {};

/**
 * @param {number} mode
 * @param {number} offset
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.drawArraysIndirect = function(mode, offset) {};

/**
 * @param {number} mode
 * @param {number} type
 * @param {number} offset
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.drawElementsIndirect = function(mode, type, offset) {};

/**
 * @param {number} target
 * @param {number} pname
 * @param {number} param
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.framebufferParameter = function(target, pname, param) {};

/**
 * @param {number} target
 * @param {number} pname
 * @return {*}
 */
WebGL2ComputeRenderingContext.prototype.getFramebufferParameter = function(target, pname) {};

/**
 * @param {!WebGLProgram} program
 * @param {number} programInterface
 * @param {number} pname
 * @return {*}
 */
WebGL2ComputeRenderingContext.prototype.getProgramInterfaceParameter = function(program, programInterface, pname) {};

/**
 * @param {!WebGLProgram} program
 * @param {number} programInterface
 * @param {number} index
 * @param {!Array<number>} props
 * @return {?Array<*>}
 */
WebGL2ComputeRenderingContext.prototype.getProgramResource = function(program, programInterface, index, props) {};

/**
 * @param {!WebGLProgram} program
 * @param {number} programInterface
 * @param {string} name
 * @return {number}
 */
WebGL2ComputeRenderingContext.prototype.getProgramResourceIndex = function(program, programInterface, name) {};

/**
 * @param {!WebGLProgram} program
 * @param {number} programInterface
 * @param {number} index
 * @return {?string}
 */
WebGL2ComputeRenderingContext.prototype.getProgramResourceName = function(program, programInterface, index) {};

/**
 * @param {!WebGLProgram} program
 * @param {number} programInterface
 * @param {string} name
 * @return {*}
 */
WebGL2ComputeRenderingContext.prototype.getProgramResourceLocation = function(program, programInterface, name) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform1i = function(program, location, v0) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform2i = function(program, location, v0, v1) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @param {number} v2
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform3i = function(program, location, v0, v1, v2) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @param {number} v2
 * @param {number} v3
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform4i = function(program, location, v0, v1, v2, v3) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform1ui = function(program, location, v0) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform2ui = function(program, location, v0, v1) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @param {number} v2
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform3ui = function(program, location, v0, v1, v2) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @param {number} v2
 * @param {number} v3
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform4ui = function(program, location, v0, v1, v2, v3) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform1f = function(program, location, v0) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform2f = function(program, location, v0, v1) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @param {number} v2
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform3f = function(program, location, v0, v1, v2) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} v0
 * @param {number} v1
 * @param {number} v2
 * @param {number} v3
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform4f = function(program, location, v0, v1, v2, v3) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Int32Array|!Array<number>|!Array<boolean>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform1iv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Int32Array|!Array<number>|!Array<boolean>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform2iv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Int32Array|!Array<number>|!Array<boolean>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform3iv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Int32Array|!Array<number>|!Array<boolean>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform4iv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Uint32Array|!Array<number>|!Array<boolean>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform1uiv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Uint32Array|!Array<number>|!Array<boolean>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform2uiv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Uint32Array|!Array<number>|!Array<boolean>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform3uiv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Uint32Array|!Array<number>|!Array<boolean>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform4uiv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform1fv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform2fv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform3fv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniform4fv = function(program, location, count, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix2fv = function(program, location, count, transpose, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix3fv = function(program, location, count, transpose, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix4fv = function(program, location, count, transpose, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix2x3fv = function(program, location, count, transpose, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix3x2fv = function(program, location, count, transpose, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix2x4fv = function(program, location, count, transpose, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix4x2fv = function(program, location, count, transpose, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix3x4fv = function(program, location, count, transpose, data) {};

/**
 * @param {?WebGLProgram} program
 * @param {?WebGLUniformLocation} location
 * @param {number} count
 * @param {boolean} transpose
 * @param {!Float32Array|!Array<number>} data
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.programUniformMatrix4x3fv = function(program, location, count, transpose, data) {};

/**
 * @param {number} unit
 * @param {?WebGLTexture} texture
 * @param {number} level
 * @param {boolean} layered
 * @param {number} layer
 * @param {number} access
 * @param {number} format
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.bindImageTexture = function(unit, texture, level, layered, layer, access, format) {};

/**
 * @param {number} barriers
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.memoryBarrier = function(barriers) {};

/**
 * @param {number} barriers
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.memoryBarrierByRegion = function(barriers) {};

/**
 * @param {number} target
 * @param {number} samples
 * @param {number} internalformat
 * @param {number} width
 * @param {number} height
 * @param {boolean} fixedsamplelocations
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.texStorage2DMultisample = function(target, samples, internalformat, width, height, fixedsamplelocations) {};

/**
 * @param {number} target
 * @param {number} level
 * @param {number} pname
 * @return {*}
 */
WebGL2ComputeRenderingContext.prototype.getTexLevelParameter = function(target, level, pname) {};

/**
 * @param {number} pname
 * @param {number} index
 * @return {*}
 */
WebGL2ComputeRenderingContext.prototype.getMultisample = function(pname, index) {};

/**
 * @param {number} index
 * @param {number} mask
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.sampleMask = function(index, mask) {};

/**
 * @param {number} bindingindex
 * @param {?WebGLBuffer} buffer
 * @param {number} offset
 * @param {number} stride
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.bindVertexBuffer = function(bindingindex, buffer, offset, stride) {};

/**
 * @param {number} attribindex
 * @param {number} size
 * @param {number} type
 * @param {boolean} normalized
 * @param {number} relativeoffset
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.vertexAttribFormat = function(attribindex, size, type, normalized, relativeoffset) {};

/**
 * @param {number} attribindex
 * @param {number} size
 * @param {number} type
 * @param {number} relativeoffset
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.vertexAttribIFormat = function(attribindex, size, type, relativeoffset) {};

/**
 * @param {number} attribindex
 * @param {number} bindingindex
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.vertexAttribBinding = function(attribindex, bindingindex) {};

/**
 * @param {number} bindingindex
 * @param {number} divisor
 * @return {undefined}
 */
WebGL2ComputeRenderingContext.prototype.vertexBindingDivisor = function(bindingindex, divisor) {};
