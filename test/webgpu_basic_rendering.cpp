// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Based on https://github.com/kainino0x/webgpu-cross-platform-demo
// (specifically on an old version that didn't use Asyncify; here we've
// intentionally kept Asyncify off to have some coverage of that case).

#include <webgpu/webgpu_cpp.h>

#undef NDEBUG
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/em_js.h>

EM_JS_DEPS(deps, "$keepRuntimeAlive");

// Keeps track of whether async tests are still alive to make sure they finish
// before exit. This tests that keepalives exist where they should.
static int sScopeCount = 0;
class ScopedCounter {
    public:
        ScopedCounter(const ScopedCounter&&) { Increment(); }
        ScopedCounter(const ScopedCounter&) { Increment(); }
        ScopedCounter() { Increment(); }
        ~ScopedCounter() { Decrement(); }
    private:
        void Increment() { sScopeCount++; }
        void Decrement() { assert(sScopeCount > 0); sScopeCount--; }
};

void RegisterCheckScopesAtExit() {
    atexit([](){
        // Check we don't exit before the tests are done.
        // (Make sure there's a keepalive for everything the test has scopes for.)
        // Build with -sRUNTIME_DEBUG to trace keepalives.
        assert(sScopeCount == 0);
    });
}

static const wgpu::Instance instance = wgpuCreateInstance(nullptr);

static const char shaderCode[] = R"(
    @vertex
    fn main_v(@builtin(vertex_index) idx: u32) -> @builtin(position) vec4<f32> {
        var pos = array<vec2<f32>, 3>(
            vec2<f32>(0.0, 0.5), vec2<f32>(-0.5, -0.5), vec2<f32>(0.5, -0.5));
        return vec4<f32>(pos[idx], 0.0, 1.0);
    }

    @fragment
    fn main_f() -> @location(0) vec4<f32> {
        return vec4<f32>(0.0, 0.502, 1.0, 1.0); // 0x80/0xff ~= 0.502
    }
)";

static wgpu::Adapter adapter;
static wgpu::Device device;
static wgpu::Queue queue;
static wgpu::Buffer readbackBuffer;
static wgpu::RenderPipeline pipeline;

void GetDevice(void (*callback)()) {
    instance.RequestAdapter(nullptr, wgpu::CallbackMode::AllowSpontaneous, [=](wgpu::RequestAdapterStatus status, wgpu::Adapter a, wgpu::StringView message) {
        if (message.length) {
            printf("RequestAdapter: %.*s\n", int(message.length), message.data);
        }
        assert(status == wgpu::RequestAdapterStatus::Success);

        wgpu::DeviceDescriptor desc;
        desc.SetUncapturedErrorCallback(
            [](const wgpu::Device&, wgpu::ErrorType errorType, wgpu::StringView message) {
                printf("UncapturedError (type=%d): %.*s\n", errorType, int(message.length), message.data);
            });

        adapter = a;
        adapter.RequestDevice(&desc, wgpu::CallbackMode::AllowSpontaneous, [=](wgpu::RequestDeviceStatus status, wgpu::Device d, wgpu::StringView message) {
            if (message.length) {
                printf("RequestDevice: %.*s\n", int(message.length), message.data);
            }
            assert(status == wgpu::RequestDeviceStatus::Success);

            device = d;
            callback();
        });
    });
}

void init() {
    queue = device.GetQueue();

    wgpu::ShaderModule shaderModule{};
    {
        wgpu::ShaderSourceWGSL wgslDesc{};
        wgslDesc.code = shaderCode;

        wgpu::ShaderModuleDescriptor descriptor{};
        descriptor.nextInChain = &wgslDesc;
        shaderModule = device.CreateShaderModule(&descriptor);
        shaderModule.GetCompilationInfo(wgpu::CallbackMode::AllowSpontaneous,
            [=](wgpu::CompilationInfoRequestStatus status, const wgpu::CompilationInfo* info) {
                assert(status == wgpu::CompilationInfoRequestStatus::Success);
                assert(info->messageCount == 0);
                printf("Shader compile succeeded\n");
            });
    }

    {
        wgpu::BindGroupLayoutDescriptor bglDesc{};
        auto bgl = device.CreateBindGroupLayout(&bglDesc);
        wgpu::BindGroupDescriptor desc{};
        desc.layout = bgl;
        desc.entryCount = 0;
        desc.entries = nullptr;
        device.CreateBindGroup(&desc);
    }

    {
        wgpu::PipelineLayoutDescriptor pl{};
        pl.bindGroupLayoutCount = 0;
        pl.bindGroupLayouts = nullptr;

        wgpu::ColorTargetState colorTargetState{};
        colorTargetState.format = wgpu::TextureFormat::BGRA8Unorm;

        wgpu::FragmentState fragmentState{};
        fragmentState.module = shaderModule;
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTargetState;

        wgpu::DepthStencilState depthStencilState{};
        depthStencilState.format = wgpu::TextureFormat::Depth32Float;
        depthStencilState.depthCompare = wgpu::CompareFunction::Always;

        wgpu::RenderPipelineDescriptor descriptor{};
        descriptor.layout = device.CreatePipelineLayout(&pl);
        descriptor.vertex.module = shaderModule;
        descriptor.fragment = &fragmentState;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.depthStencil = &depthStencilState;
        pipeline = device.CreateRenderPipeline(&descriptor);
    }
}

// The depth stencil attachment isn't really needed to draw the triangle
// and doesn't really affect the render result.
// But having one should give us a slightly better test coverage for the compile of the depth stencil descriptor.
void render(wgpu::TextureView view, wgpu::TextureView depthStencilView) {
    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = view;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    attachment.clearValue = {0, 0, 0, 1};

    wgpu::RenderPassDescriptor renderpass{};
    renderpass.colorAttachmentCount = 1;
    renderpass.colorAttachments = &attachment;

    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
    depthStencilAttachment.view = depthStencilView;
    depthStencilAttachment.depthClearValue = 0;
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;

    renderpass.depthStencilAttachment = &depthStencilAttachment;

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);
            pass.SetPipeline(pipeline);
            pass.Draw(3);
            pass.End();
        }
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);
}

void issueContentsCheck(ScopedCounter scope, std::string functionName, wgpu::Buffer readbackBuffer, uint32_t expectData) {
    readbackBuffer.MapAsync(
        wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowSpontaneous,
        [=, scope=scope](wgpu::MapAsyncStatus status, wgpu::StringView message) {
            if (message.length) {
                printf("readbackBuffer.MapAsync: %.*s\n", int(message.length), message.data);
            }
            assert(status == wgpu::MapAsyncStatus::Success);

            const void* ptr = readbackBuffer.GetConstMappedRange();

            printf("%s: readback -> %p%s\n", functionName.c_str(),
                    ptr, ptr ? "" : " <------- FAILED");
            assert(ptr != nullptr);
            uint32_t readback = static_cast<const uint32_t*>(ptr)[0];
            readbackBuffer.Unmap();
            printf("  got %08x, expected %08x%s\n",
                readback, expectData,
                readback == expectData ? "" : " <------- FAILED");
        });
}

void doCopyTestMappedAtCreation(ScopedCounter scope, bool useRange) {
    static constexpr uint32_t kValue = 0x05060708;
    size_t size = useRange ? 12 : 4;
    wgpu::Buffer src;
    {
        wgpu::BufferDescriptor descriptor{};
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::CopySrc;
        descriptor.mappedAtCreation = true;
        src = device.CreateBuffer(&descriptor);
    }
    size_t offset = useRange ? 8 : 0;
    uint32_t* ptr = static_cast<uint32_t*>(useRange ?
            src.GetMappedRange(offset, 4) :
            src.GetMappedRange());
    printf("%s: getMappedRange -> %p%s\n", __FUNCTION__,
            ptr, ptr ? "" : " <------- FAILED");
    assert(ptr != nullptr);
    *ptr = kValue;
    src.Unmap();

    wgpu::Buffer dst;
    {
        wgpu::BufferDescriptor descriptor{};
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
        dst = device.CreateBuffer(&descriptor);
    }

    // Write some random data to the buffer, just to verify that
    // wgpuQueueWriteBuffer works.
    char data[4];
    queue.WriteBuffer(dst, 0, data, sizeof(data));

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(src, offset, dst, 0, 4);
        commands = encoder.Finish();
    }
    queue.Submit(1, &commands);

    issueContentsCheck(scope, __FUNCTION__, dst, kValue);
}

void doCopyTestMapAsync(ScopedCounter scope, bool useRange) {
    static constexpr uint32_t kValue = 0x01020304;
    size_t size = useRange ? 12 : 4;
    wgpu::Buffer src;
    {
        wgpu::BufferDescriptor descriptor{};
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        src = device.CreateBuffer(&descriptor);
    }
    size_t offset = useRange ? 8 : 0;

    std::string functionName = __FUNCTION__;
    src.MapAsync(
        wgpu::MapMode::Write, offset, 4, wgpu::CallbackMode::AllowSpontaneous,
        [=](wgpu::MapAsyncStatus status, wgpu::StringView message) {
            if (message.length) {
                printf("src.MapAsync: %.*s\n", int(message.length), message.data);
            }
            assert(status == wgpu::MapAsyncStatus::Success);

            uint32_t* ptr = static_cast<uint32_t*>(useRange ?
                    src.GetMappedRange(offset, 4) :
                    src.GetMappedRange());
            printf("%s: getMappedRange -> %p%s\n", functionName.c_str(),
                    ptr, ptr ? "" : " <------- FAILED");
            assert(ptr != nullptr);
            *ptr = kValue;
            src.Unmap();

            wgpu::Buffer dst;
            {
                wgpu::BufferDescriptor descriptor{};
                descriptor.size = 4;
                descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
                dst = device.CreateBuffer(&descriptor);
            }

            wgpu::CommandBuffer commands;
            {
                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                encoder.CopyBufferToBuffer(src, offset, dst, 0, 4);
                commands = encoder.Finish();
            }
            queue.Submit(1, &commands);

            issueContentsCheck(scope, functionName, dst, kValue);
        });
}

void doRenderTest(ScopedCounter scope) {
    wgpu::Texture readbackTexture;
    {
        wgpu::TextureDescriptor descriptor{};
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        descriptor.size = {1, 1, 1};
        descriptor.format = wgpu::TextureFormat::BGRA8Unorm;

        // Test for viewFormats binding
        std::array<wgpu::TextureFormat, 2> viewFormats =
            { wgpu::TextureFormat::BGRA8Unorm, wgpu::TextureFormat::BGRA8Unorm };
        descriptor.viewFormatCount = viewFormats.size();
        descriptor.viewFormats = viewFormats.data();

        readbackTexture = device.CreateTexture(&descriptor);
    }
    wgpu::Texture depthTexture;
    {
        wgpu::TextureDescriptor descriptor{};
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
        descriptor.size = {1, 1, 1};
        descriptor.format = wgpu::TextureFormat::Depth32Float;
        depthTexture = device.CreateTexture(&descriptor);
    }
    render(readbackTexture.CreateView(), depthTexture.CreateView());

    {
        // A little texture.GetFormat test
        assert(wgpu::TextureFormat::BGRA8Unorm == readbackTexture.GetFormat());
        assert(wgpu::TextureFormat::Depth32Float == depthTexture.GetFormat());
    }

    {
        wgpu::BufferDescriptor descriptor{};
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;

        readbackBuffer = device.CreateBuffer(&descriptor);
    }

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo src{};
        src.texture = readbackTexture;
        src.origin = {0, 0, 0};
        wgpu::TexelCopyBufferInfo dst{};
        dst.buffer = readbackBuffer;
        dst.layout.bytesPerRow = 256;
        wgpu::Extent3D extent = {1, 1, 1};
        encoder.CopyTextureToBuffer(&src, &dst, &extent);
        commands = encoder.Finish();
    }
    queue.Submit(1, &commands);

    // Check the color value encoded in the shader makes it out correctly.
    static const uint32_t expectData = 0xff0080ff;
    issueContentsCheck(scope, __FUNCTION__, readbackBuffer, expectData);
}

wgpu::Surface surface;
wgpu::TextureView canvasDepthStencilView;
const uint32_t kWidth = 300;
const uint32_t kHeight = 150;

void frame(void* vp_scope) {
    auto scope = std::unique_ptr<ScopedCounter>(reinterpret_cast<ScopedCounter*>(vp_scope));

    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);

    wgpu::TextureView backbuffer = surfaceTexture.texture.CreateView();
    render(backbuffer, canvasDepthStencilView);

    // Test should complete when runtime exists after all async work is done.
    emscripten_cancel_main_loop();
}

void run() {
    init();

    ScopedCounter scope;
    doCopyTestMappedAtCreation(scope, false);
    doCopyTestMappedAtCreation(scope, true);
    doCopyTestMapAsync(scope, false);
    doCopyTestMapAsync(scope, true);
    doRenderTest(scope);

    {
        wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc{};
        canvasDesc.selector = "#canvas";

        wgpu::SurfaceDescriptor surfDesc{};
        surfDesc.nextInChain = &canvasDesc;
        surface = instance.CreateSurface(&surfDesc);

        wgpu::SurfaceCapabilities capabilities;
        surface.GetCapabilities(adapter, &capabilities);

        wgpu::SurfaceConfiguration config{
            .device = device,
            .format = capabilities.formats[0],
            .usage = wgpu::TextureUsage::RenderAttachment,
            .width = kWidth,
            .height = kHeight,
            .alphaMode = wgpu::CompositeAlphaMode::Auto,
            .presentMode = wgpu::PresentMode::Fifo};
        surface.Configure(&config);

        {
            wgpu::TextureDescriptor descriptor{};
            descriptor.usage = wgpu::TextureUsage::RenderAttachment;
            descriptor.size = {kWidth, kHeight, 1};
            descriptor.format = wgpu::TextureFormat::Depth32Float;
            canvasDepthStencilView = device.CreateTexture(&descriptor).CreateView();
        }
    }

    emscripten_set_main_loop_arg(frame, new ScopedCounter(), 0, false);
}

int main() {
    GetDevice(run);

    RegisterCheckScopesAtExit();
    // This is the return code once all runtime-keepalives have completed
    // (unless something crashes before then).
    return 0;
}
