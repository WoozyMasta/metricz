# Telemetry

## Purpose

This mod is **server-side only** and can be distributed via multiple channels.
Telemetry is used only to understand whether the project is actively used.

## Data Sent

Minimal and non-identifying:

* Application name
* Version
* Distribution type
* Server port
* Disabled automatically in `#ifdef DIAG`

Example:

```json
{
  "application": "MetricZ",
  "version": "1.1.0",
  "type": "steam",
  "port": 27016
}
```

## Behavior

* One request per server restart
* Sent to <https://zenit.woozymasta.ru/> 10–20 minutes after startup
* No personal or player data

## Disable Telemetry

* **`settings.disable_telemetry`** (`bool`)
  When `true`, telemetry is completely disabled.

```json
{
  "settings": {
    "disable_telemetry": true
  }
}
```
