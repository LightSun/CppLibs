#include "perfetto.h"

//https://perfetto.dev/docs/instrumentation/tracing-sdk
PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("rendering")
        .SetDescription("Events from the graphics subsystem"),
    perfetto::Category("network")
        .SetDescription("Network upload and download statistics"));

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

class CustomDataSource : public perfetto::DataSource<CustomDataSource> {
 public:
  void OnSetup(const SetupArgs&) override {
    // Use this callback to apply any custom configuration to your data source
    // based on the TraceConfig in SetupArgs.
  }

  void OnStart(const StartArgs&) override {
    // This notification can be used to initialize the GPU driver, enable
    // counters, etc. StartArgs will contains the DataSourceDescriptor,
    // which can be extended.
  }

  void OnStop(const StopArgs&) override {
    // Undo any initialization done in OnStart.
  }

  // Data sources can also have per-instance state.
  int my_custom_state = 0;
};

PERFETTO_DECLARE_DATA_SOURCE_STATIC_MEMBERS(CustomDataSource);


int main(int argc, const char* argv[]){

    perfetto::TracingInitArgs args;

    // The backends determine where trace events are recorded. You may select one
    // or more of:

    // 1) The in-process backend only records within the app itself.
    args.backends |= perfetto::kInProcessBackend;

    // 2) The system backend writes events into a system Perfetto daemon,
    //    allowing merging app and system events (e.g., ftrace) on the same
    //    timeline. Requires the Perfetto `traced` daemon to be running (e.g.,
    //    on Android Pie and newer).
    args.backends |= perfetto::kSystemBackend;

    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();
    //
    perfetto::DataSourceDescriptor dsd;
     dsd.set_name("com.example.custom_data_source");
     CustomDataSource::Register(dsd);

     //after all data-source registered. enable tracing
     perfetto::TraceConfig cfg;
     auto* ds_cfg = cfg.add_data_sources()->mutable_config();
     ds_cfg->set_name("com.example.custom_data_source");

     //call the Trace() method to record an event with your custom data source
     CustomDataSource::Trace([](CustomDataSource::TraceContext ctx) {
       auto packet = ctx.NewTracePacket();
       packet->set_timestamp(perfetto::TrackEvent::GetTraceTimeNs());
       packet->set_for_testing()->set_str("Hello world!");
     });

     //access the custom data source state. take a mutex to ensure data source isn't destroyed
     CustomDataSource::Trace([](CustomDataSource::TraceContext ctx) {
       auto safe_handle = ctx.GetDataSourceLocked();  // Holds a RAII lock.
       //DoSomethingWith(safe_handle->my_custom_state);
     });
    return 0;
}

void LayerTreeHost_DoUpdateLayers() {
  TRACE_EVENT("rendering", "LayerTreeHost::DoUpdateLayers");
  //...
//  for (PictureLayer& pl : layers) {
//    TRACE_EVENT("rendering", "PictureLayer::Update");
//    pl.Update();
//  }
}

//void LoadGame() {
//  DisplayLoadingScreen();

//  TRACE_EVENT_BEGIN("io", "Loading");  // Begin "Loading" slice.
//  LoadCollectibles();
//  LoadVehicles();
//  LoadPlayers();
//  TRACE_EVENT_END("io");               // End "Loading" slice.

//  StartGame();
//}

