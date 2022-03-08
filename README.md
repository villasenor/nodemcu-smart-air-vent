# nodemcu-smart-air-vent
NodeMCU Controlled Smart Vent for Home Assistant

I'll post an Instructable and more information on how I built this soon! I wanted an automatic way to control HVAC air for specific rooms that get hotter or colder than the rest of the house (single zone), so I built this! There are commercial versions of this, but they depend on the cloud and are much more expensive. This project integrates locally with Home Assistant via REST. Home Assistant has several temperature sensors attached to it. They are used to automatically determine when the vent needs to be opened or closed.

My Home Assistant config looked something like this:
```
  - platform: rest
    name: "Smart Vent"
    resource: http://<NODEMCU_IP>/control
    state_resource: http://<NODEMCU_IP>/status
    body_on: '{"active": "true"}'
    body_off: '{"active": "false"}'
    is_on_template: '{{ value_json.is_active }}'
    headers:
      Content-Type: application/json
```
Used NodeMCU v1, L9110 motor controller, and an old VentMiser Vent I found on eBay (you can just wire the motor controller directly to the motor on the vent - the vent opens and closes at 3.3v, but slightly higher voltages for short amounts of time seemed to be fine)
