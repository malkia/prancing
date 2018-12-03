#include "opencensus/trace/span.h"
#include "opencensus/exporters/stats/stdout/stdout_exporter.h"
#include "opencensus/exporters/trace/stdout/stdout_exporter.h"
#include "opencensus/exporters/trace/zipkin/zipkin_exporter.h"

#include <thread>

void ParallelWorker(::opencensus::trace::Span* span, int work_unit) 
{
  auto msg = std::string( "Doing work unit" + std::to_string(work_unit) + " on behaf of currently installed context." );
  span->AddAnnotation( msg );
  auto op_span = ::opencensus::trace::Span::StartSpan("Working", span);
  op_span.End();
}

void happy(int i)
{
  static opencensus::trace::AlwaysSampler sampler;
  auto op_span = ::opencensus::trace::Span::StartSpan("HappyPonny", nullptr, {&sampler});
  {
    std::thread t1(ParallelWorker, &op_span, i*2 + 0);
    std::thread t2(ParallelWorker, &op_span, i*2 + 1);
    t1.join();
    t2.join();
  }
  op_span.End();
}

int main( int argc, char *argv[] )
{
  ::opencensus::exporters::stats::StdoutExporter::Register();
  ::opencensus::exporters::trace::StdoutExporter::Register();

  ::opencensus::exporters::trace::ZipkinExporterOptions options("http://100.115.92.200:9411/api/v2/spans");
  options.af_type = ::opencensus::exporters::trace::ZipkinExporterOptions::AddressFamily::kIpv4;
  options.service_name = "TestService";
  ::opencensus::exporters::trace::ZipkinExporter::Register(options);

  for( int i=0; i<1000; i++ )
    happy(i);

  return 0;
}
