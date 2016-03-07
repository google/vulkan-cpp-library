/*
* Copyright 2016 Google Inc. All Rights Reserved.

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <fstream>
#include <iostream>
#include <vcc/util.h>

#include <vcc/instance.h>
#include <vcc/device.h>
#include <vcc/shader_module.h>
#include <vcc/physical_device.h>


/*int main(int argc, const char **argv) {

	auto instance(vcc::instance::create({}, {}));
	auto device_enumerate(vcc::physical_device::enumerate(std::ref(instance)));
	auto device(std::make_shared<vcc::device::device_type>(vcc::device::create(device_enumerate.front(), {}, {}, {},
			vcc::physical_device::features(device_enumerate.front()))));
	auto shader_module(vcc::shader_module::create(device, std::ifstream("shader.file")));
	return 0;
}
*/
