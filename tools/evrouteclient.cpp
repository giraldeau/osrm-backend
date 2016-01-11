/*

Copyright (c) 2015, Project OSRM contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "util/version.hpp"
#include "../util/json_renderer.hpp"
#include "../util/routed_options.hpp"
#include "../util/simple_logger.hpp"

#include <osrm/json_container.hpp>
#include <osrm/libosrm_config.hpp>
#include <osrm/route_parameters.hpp>
#include <osrm/osrm.hpp>

#include <string>
#include <functional>

int main(int argc, const char *argv[])
{
    LogPolicy::GetInstance().Unmute();
    try
    {
        std::string ip_address;
        int ip_port, requested_thread_num;
        bool trial_run = false;
        LibOSRMConfig conf;
        const unsigned init_result = GenerateServerProgramOptions(
            argc, argv, conf.server_paths, ip_address, ip_port, requested_thread_num,
            conf.use_shared_memory, trial_run, conf.max_locations_trip,
            conf.max_locations_viaroute, conf.max_locations_distance_table,
            conf.max_locations_map_matching);

        if (init_result == INIT_OK_DO_NOT_START_ENGINE)
        {
            return 0;
        }
        if (init_result == INIT_FAILED)
        {
            return 1;
        }
        SimpleLogger().Write() << "starting up engines, " << OSRM_VERSION;

        OSRM routing_machine(conf);

        RouteParameters req;
        req.zoom_level = 18;           // no generalization
        req.print_instructions = true; // turn by turn instructions
        req.alternate_route = false;    // get an alternate route, too
        req.geometry = true;           // retrieve geometry of route
        req.compression = true;        // polyline encoding
        req.check_sum = -1;            // see wiki
        req.service = "evroute";      // that's routing
        req.output_format = "json";
        req.jsonp_parameter = ""; // set for jsonp wrapping
        req.language = "";        // unused atm
        // route_parameters.hints.push_back(); // see wiki, saves I/O if done properly

        // start_coordinate
        std::vector<std::vector<double> > ways = {{45.56028, -73.8519557}, {46.8581563, -71.4864987}};
        std::for_each(ways.begin(), ways.end(), [&](std::vector<double> &way) {
            req.coordinates.emplace_back(way[0] * COORDINATE_PRECISION, way[1] * COORDINATE_PRECISION);
        });
        osrm::json::Object json_result;
        const int result_code = routing_machine.RunQuery(req, json_result);
        SimpleLogger().Write() << "http code: " << result_code;
        osrm::json::render(SimpleLogger().Write(), json_result);
    }
    catch (std::exception &current_exception)
    {
        SimpleLogger().Write(logWARNING) << "caught exception: " << current_exception.what();
        return -1;
    }
    return 0;
}
