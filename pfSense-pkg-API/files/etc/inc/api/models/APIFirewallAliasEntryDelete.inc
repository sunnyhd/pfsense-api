<?php
//   Copyright 2020 Jared Hendrickson
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

require_once("api/framework/APIModel.inc");
require_once("api/framework/APIResponse.inc");

class APIFirewallAliasEntryDelete extends APIModel {
    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->privileges = ["page-all", "page-firewall-aliases-edit"];
        $this->change_note = "Deleted firewall alias address via API";
    }

    public function action() {
        $this->config["aliases"]["alias"][$this->validated_data["id"]]["address"] = implode(" ", $this->validated_data["address"]);
        $this->config["aliases"]["alias"][$this->validated_data["id"]]["detail"] = implode("||", $this->validated_data["detail"]);
        $this->write_config();    // Apply our configuration change
        send_event("filter reload");    // Ensure our firewall filter is reloaded
        return APIResponse\get(0, $this->config["aliases"]["alias"][$this->validated_data["id"]]);
    }
    
    public function validate_payload() {

        if (isset($this->initial_data['name'])) {
            $name = $this->initial_data['name'];
            $name = APITools\sanitize_str($name);
        } else {
            $this->errors[] = APIResponse\get(4050);
        }
        if (isset($this->initial_data['address'])) {
            $address = $this->initial_data['address'];
            // Convert string to array
            if (!is_array($address)) {
                $address = array($address);
            }
        } else {
            $this->errors[] = APIResponse\get(4052);
        }
        // Check that our input is valid
        if (!is_string($name)) {
            $this->errors[] = APIResponse\get(4053);
        } elseif (!is_array($address)) {
            $this->errors[] = APIResponse\get(4054);
        }
        // Loop through our existing firewall entries and check for our requested alias
        $c_count = 0;
        foreach ($this->config["aliases"]["alias"] as $ce) {
            if ($name === $ce["name"]) {
                $alias_found = true;
                $this->validated_data["id"] = $c_count;
                $this->validated_data["address"] = explode(" ", $ce["address"]);
                $this->validated_data["detail"] = explode("||", $ce["detail"]);
                break;
            }
            $c_count++;    // Increase our counter
        }
        // If we could not find an alias, return error
        if ($alias_found !== true) {
            $this->errors[] = APIResponse\get(4055);
        }

        // Loop through our existing configuration and remove alias address values on match
        $r_count = 0;
        foreach ($this->validated_data["address"] as $re) {
            if (in_array($re, $address)) {
                unset($this->validated_data["address"][$r_count]);
                unset($this->validated_data["detail"][$r_count]);
            }
            $r_count++;    // Increase our counter
        }

    }
}