// Copyright 2015 Open vStorage NV
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "FileSystemEventTestSetup.h"

#include <thread>

#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <youtils/Logging.h>
#include <youtils/TestBase.h>
#include <youtils/UUID.h>
#include <thread>

#include "../EventPublisher.h"
#include "../FileSystemEvents.h"
#include "../FileSystemEvents.pb.h"
#include "../FileSystemParameters.h"
#include "../FrontendPath.h"

namespace volumedriverfstest
{

namespace bpt = boost::property_tree;
namespace fs = boost::filesystem;
namespace ip = initialized_params;
namespace vd = volumedriver;
namespace vfs = volumedriverfs;
namespace yt = youtils;
namespace ytt = youtilstest;

class FileSystemEventTest
    : public ytt::TestBase
    , public FileSystemEventTestSetup
{
protected:
    FileSystemEventTest()
        : FileSystemEventTestSetup("FileSystemEventTest")
    {}

    virtual ~FileSystemEventTest()
    {}

    DECLARE_LOGGER("FileSystemEventTest");
};

TEST_F(FileSystemEventTest, volume_create)
{
    if (use_amqp())
    {
        const vfs::FrontendPath vpath("/volume");
        const vd::VolumeId vid("volume");
        const uint64_t vsize = ~0ULL;

        publish_event(vfs::FileSystemEvents::volume_create(vid,
                                                           vpath,
                                                           vsize));

        const events::VolumeCreateMessage msg(get_event(events::volume_create));
        EXPECT_TRUE(msg.has_name());
        EXPECT_EQ(vid.str(), msg.name());

        EXPECT_TRUE(msg.has_path());
        EXPECT_EQ(vpath.str(), msg.path());

        EXPECT_TRUE(msg.has_size());
        EXPECT_EQ(vsize, msg.size());
    }
    else
    {
        LOG_WARN("Not running test as AMQP is not configured");
    }
}

TEST_F(FileSystemEventTest, volume_delete)
{
    if (use_amqp())
    {
        const vfs::FrontendPath vpath("/volume");
        const vd::VolumeId vid("volume");

        publish_event(vfs::FileSystemEvents::volume_delete(vid,
                                                           vpath));

        const events::VolumeDeleteMessage msg(get_event(events::volume_delete));
        EXPECT_TRUE(msg.has_name());
        EXPECT_EQ(vid.str(), msg.name());

        EXPECT_TRUE(msg.has_path());
        EXPECT_EQ(vpath.str(), msg.path());
    }
    else
    {
        LOG_WARN("Not running test as AMQP is not configured");
    }
}

TEST_F(FileSystemEventTest, volume_rename)
{
    if (use_amqp())
    {
        const vfs::FrontendPath from("/volume-old");
        const vfs::FrontendPath to("/volume-old");
        const vd::VolumeId vid("volume");

        publish_event(vfs::FileSystemEvents::volume_rename(vid,
                                                           from,
                                                           to));

        const events::VolumeRenameMessage msg(get_event(events::volume_rename));
        EXPECT_TRUE(msg.has_name());
        EXPECT_EQ(vid.str(), msg.name());

        EXPECT_TRUE(msg.has_old_path());
        EXPECT_EQ(from.str(), msg.old_path());

        EXPECT_TRUE(msg.has_new_path());
        EXPECT_EQ(to.str(), msg.new_path());
    }
    else
    {
        LOG_WARN("Not running test as AMQP is not configured");
    }
}

TEST_F(FileSystemEventTest, volume_resize)
{
    if (use_amqp())
    {
        const vfs::FrontendPath vpath("/volume");
        const vd::VolumeId vid("volume");
        const uint64_t vsize = ~0ULL;

        publish_event(vfs::FileSystemEvents::volume_resize(vid,
                                                           vpath,
                                                           vsize));

        const events::VolumeResizeMessage msg(get_event(events::volume_resize));

        EXPECT_TRUE(msg.has_name());
        EXPECT_EQ(vid.str(), msg.name());

        EXPECT_TRUE(msg.has_path());
        EXPECT_EQ(vpath.str(), msg.path());

        EXPECT_TRUE(msg.has_size());
        EXPECT_EQ(vsize, msg.size());
    }
    else
    {
        LOG_WARN("Not running test as AMQP is not configured");
    }
}

TEST_F(FileSystemEventTest, volume_create_failed)
{
    if (use_amqp())
    {
        const vfs::FrontendPath vpath("/volume");
        const std::string reason("for no particularly good reason");

        publish_event(vfs::FileSystemEvents::volume_create_failed(vpath,
                                                                  reason));

        const events::VolumeCreateFailedMessage
            msg(get_event(events::volume_create_failed));

        EXPECT_TRUE(msg.has_path());
        EXPECT_EQ(vpath.str(), msg.path());

        EXPECT_TRUE(msg.has_reason());
        EXPECT_EQ(reason, msg.reason());
    }
    else
    {
        LOG_WARN("Not running test as AMQP is not configured");
    }
}

TEST_F(FileSystemEventTest, multithreaded)
{
    if (use_amqp())
    {
        std::mutex lock;
        std::set<std::string> events;
        const unsigned nthreads = 4;
        std::vector<std::thread> threads;
        threads.reserve(nthreads);
        const unsigned nevents = 1000;

        for (unsigned i = 0; i < nthreads; ++i)
        {
            auto f([&, i]
                   {
                       for (unsigned j = 0; j < nevents; ++j)
                       {
                           const std::string
                               s("ev-" +
                                 boost::lexical_cast<std::string>(i) +
                                 vfs::AmqpUri("-") +
                                 boost::lexical_cast<std::string>(j));

                           const auto
                               ev(vfs::FileSystemEvents::volume_create(vd::VolumeId(s),
                                                                       vfs::FrontendPath(s),
                                                                       0));

                           publish_event(ev);

                           std::lock_guard<std::mutex> g(lock);
                           const auto res(events.insert(s));
                           EXPECT_TRUE(res.second);
                       }
                   });

            threads.emplace_back(std::move(f));
        }

        for (unsigned i = 0; i < nthreads; ++i)
        {
            threads[i].join();
        }

        EXPECT_EQ(nthreads * nevents, events.size());

        for (unsigned i = 0; i < nthreads * nevents; ++i)
        {
            auto msg(get_event(events::volume_create));
            EXPECT_EQ(1U, events.erase(msg.name()));
        }

        EXPECT_TRUE(events.empty());
    }
    else
    {
        LOG_WARN("Not running test as AMQP is not configured");
    }
}

TEST_F(FileSystemEventTest, invalid_uri)
{
    const std::vector<vfs::AmqpUri> invalid_uri = { vfs::AmqpUri("ampq:\\localhost") };
    const vfs::AmqpExchange exchange("exchange");
    const vfs::AmqpRoutingKey rkey("queue");

    bpt::ptree pt;
    FileSystemEventTestSetup::make_config(pt, invalid_uri, exchange, rkey);

    EXPECT_THROW(vfs::EventPublisher p(cluster_id(),
                                       node_id(),
                                       pt),
                 std::exception);
}

TEST_F(FileSystemEventTest, no_one_listening)
{
    const std::vector<vfs::AmqpUri> uris = {
        vfs::AmqpUri("amqp://localshot"),
        vfs::AmqpUri("amqp://coalsloth")
    };

    const vfs::AmqpExchange exchange("exchange");
    const vfs::AmqpRoutingKey rkey("queue");

    bpt::ptree pt;
    FileSystemEventTestSetup::make_config(pt, uris, exchange, rkey);

    vfs::EventPublisher p(cluster_id(),
                          node_id(),
                          pt);

    const vd::VolumeId vid("volume-name");
    const vfs::FrontendPath vpath("/volume-path");

    p.publish(vfs::FileSystemEvents::volume_delete(vid,
                                                   vpath));
}

TEST_F(FileSystemEventTest, cluster_of_uris)
{
    if (use_amqp())
    {
        const std::vector<vfs::AmqpUri> uris = {
            vfs::AmqpUri("amqp://localshot"),
            vfs::AmqpUri("amqp://coalsloth"),
            amqp_uri()
        };

        bpt::ptree pt;
        FileSystemEventTestSetup::make_config(pt,
                                    uris,
                                    amqp_exchange(),
                                    amqp_routing_key());

        vfs::EventPublisher p(cluster_id(),
                              node_id(),
                              pt);

        p.publish(vfs::FileSystemEvents::volume_delete(vd::VolumeId("volume-name"),
                                                       vfs::FrontendPath("/volume-path")));
    }
    else
    {
        LOG_WARN("Not running test as AMQP is not configured");
    }
}

TEST_F(FileSystemEventTest, simple_event_serialization_roundtrip)
{
    const std::string some_mountpoint("//some//mountpoint");
    const events::Event ev_out(vfs::FileSystemEvents::up_and_running(some_mountpoint));

    const std::string s(ev_out.SerializeAsString());

    events::Event ev_in;
    ev_in.ParseFromString(s);

    ASSERT_TRUE(ev_in.HasExtension(events::up_and_running));
    const auto ext(ev_in.GetExtension(events::up_and_running));

    EXPECT_TRUE(ext.has_mountpoint());
    EXPECT_EQ(some_mountpoint, ext.mountpoint());
}

TEST_F(FileSystemEventTest, config_check)
{
    auto fun([&](const std::initializer_list<vfs::AmqpUri>& uril) -> bool
             {
                 const std::vector<vfs::AmqpUri> uriv(uril);
                 bpt::ptree pt;
                 const ip::PARAMETER_TYPE(events_amqp_uris) uris(uriv);
                 uris.persist(pt);

                 yt::ConfigurationReport crep;
                 bool ok = publisher_->checkConfig(pt,
                                                   crep);

                 if (not ok)
                 {
                     EXPECT_EQ(1U, crep.size());
                     const yt::ConfigurationProblem& prob(crep.front());

                     EXPECT_EQ(uris.name(),
                               prob.param_name());

                     EXPECT_EQ(uris.section_name(),
                               prob.component_name());
                 }
                 else
                 {
                     EXPECT_TRUE(crep.empty());
                 }

                 return ok;
             });

    EXPECT_TRUE(fun({}));
    EXPECT_TRUE(fun({ vfs::AmqpUri("amqp://localhost") }));
    EXPECT_TRUE(fun({ vfs::AmqpUri("amqp://localhost"),
                      vfs::AmqpUri("amqp://localhost:9"),
                      vfs::AmqpUri("amqp://user:pass@localhost:9"),
                      vfs::AmqpUri("amqp://user:pass@127.0.0.1:9") }));
    EXPECT_FALSE(fun({ vfs::AmqpUri("amqp:\\localhost") }));
    EXPECT_FALSE(fun({ vfs::AmqpUri("amqp://localhost"),
                       vfs::AmqpUri("amqp:\\coalsloth") }));
}

TEST_F(FileSystemEventTest, config_update)
{
    bpt::ptree pt;
    publisher_->persist(pt,
                        ReportDefault::F);

    const ip::PARAMETER_TYPE(events_amqp_uris) uris(pt);

    if (use_amqp())
    {
        ASSERT_EQ(1U, uris.value().size());
        EXPECT_EQ(amqp_uri().str(),
                  uris.value()[0]);
    }
    else
    {
        ASSERT_TRUE(uris.value().empty());
    }

    const ip::PARAMETER_TYPE(events_amqp_exchange) exchange(pt);
    EXPECT_EQ(amqp_exchange().str(),
              exchange.value());

    const ip::PARAMETER_TYPE(events_amqp_routing_key) key(pt);
    EXPECT_EQ(amqp_routing_key().str(),
              key.value());

    auto test_publish([&](bool enabled)
                      {
                          const vfs::FrontendPath vpath("/volume");
                          const vd::VolumeId vid(yt::UUID().str());
                          const uint64_t vsize = ~0ULL;

                          publish_event(vfs::FileSystemEvents::volume_create(vid,
                                                                             vpath,
                                                                             vsize));

                          if (enabled)
                          {
                              const events::VolumeCreateMessage
                                  msg(get_event(events::volume_create));
                              EXPECT_TRUE(msg.has_name());
                              EXPECT_EQ(vid.str(), msg.name());
                              EXPECT_TRUE(msg.has_path());
                              EXPECT_EQ(vpath.str(), msg.path());

                              EXPECT_TRUE(msg.has_size());
                              EXPECT_EQ(vsize, msg.size());
                          }
                      });

    test_publish(use_amqp());

    const vfs::AmqpUri new_uri("amqp://localhost:9"); // discard service
    const std::vector<vfs::AmqpUri> new_uris{ new_uri };
    const vfs::AmqpExchange new_exchange("exchange");
    ASSERT_NE(exchange.value(),
              new_exchange);

    const vfs::AmqpRoutingKey new_key("key");
    ASSERT_NE(exchange.value(),
              new_exchange);

    decltype(uris)(new_uris).persist(pt);
    decltype(exchange)(new_exchange).persist(pt);
    decltype(key)(new_key).persist(pt);

    {
        yt::UpdateReport urep;

        publisher_->update(pt,
                           urep);

        ASSERT_EQ(3U,
                  urep.update_size());
    }

    test_publish(false);

    publisher_->persist(pt,
                        ReportDefault::F);

    EXPECT_EQ(new_uris,
              decltype(uris)(pt).value());
    EXPECT_EQ(new_exchange,
              decltype(exchange)(pt).value());
    EXPECT_EQ(new_key,
              decltype(key)(pt).value());

    uris.persist(pt);
    exchange.persist(pt);
    key.persist(pt);

    yt::UpdateReport urep;

    publisher_->update(pt,
                       urep);

    ASSERT_EQ(3U,
              urep.update_size());

    test_publish(use_amqp());
}

}
