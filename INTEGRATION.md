# Using MetricZ in Your Own Mods

MetricZ can be used as a metrics library inside other mods.
Dependency is optional: wrap all calls in `#ifdef METRICZ`.

## Compile-time guard

```cpp
#ifdef METRICZ
// Code using MetricZ classes
#endif
```

If MetricZ is not loaded, the block is skipped.

## Core metric types

```cpp
class MetricZ_MetricInt;
class MetricZ_MetricFloat;
class MetricZ_MetricBase;
enum MetricZ_MetricType { GAUGE, COUNTER }
```

Metric names are provided *without* the `dayz_metricz_` prefix —
it is added automatically.

```cpp
#ifdef METRICZ
static ref MetricZ_MetricInt s_MyModJumps = new MetricZ_MetricInt(
  "my_mod_jumps",
  "Total player jumps triggered by my mod",
  MetricZ_MetricType.COUNTER
);
#endif
```

## Labels

Static label example:

```cpp
#ifdef METRICZ
static ref MetricZ_MetricInt s_MyItemUses = new MetricZ_MetricInt(
  "my_item_uses",
  "MySuperItem use count",
  MetricZ_MetricType.COUNTER
);

static void InitMyMetrics()
{
  s_MyItemUses.MakeLabel("item", "MySuperItem");
}
#endif
```

Base labels added automatically:

* `world`
* `host`
* `instance_id`

Additional labels:

* `metric.MakeLabel("key","value")`
* `metric.MakeLabels(map<string,string>)`

Keep label cardinality controlled.

## Registering metrics from another mod

You can extend MetricZ by modifying its storage class via
`modded class MetricZ_Storage`.

Pattern:

```cpp
modded class MetricZ_Storage
{
#ifdef METRICZ
  protected static ref MetricZ_MetricInt s_MyCustomMetric = new MetricZ_MetricInt(
    "my_custom_metric",
    "Description of my custom metric",
    MetricZ_MetricType.COUNTER
  );

  override static void Init()
  {
    super.Init(); // run MetricZ built-in initialization
    s_Registry.Insert(s_MyCustomMetric); // register your metric
  }
#endif
}
```

After insertion into `s_Registry`, the metric is automatically written into
`metricz.prom` on each scrape.

<!-- Works only from 1.29 game version -->
This allows clean optional integration without patching MetricZ code.

## Examples

### Counting player jumps

```cpp
modded class MetricZ_Storage
{
#ifdef METRICZ
  protected static ref MetricZ_MetricInt s_PlayerJumps = new MetricZ_MetricInt(
    "player_jumps",
    "Total player jump starts",
    MetricZ_MetricType.COUNTER
  );

  override static void Init()
  {
    super.Init();
    s_Registry.Insert(s_PlayerJumps);
  }
#endif
}
```

Use it in gameplay code:

```cpp
modded class PlayerBase
{
#ifdef METRICZ
  override void OnJumpStart()
  {
    MetricZ_Storage.s_PlayerJumps.Inc();
    super.OnJumpStart();
  }
#endif
}
```

Resulting metric:

```PromQL
dayz_metricz_player_jumps_total{world="..",host="..",instance_id=".."} N
```

### Counting interactions with a custom mod item

Register the metric:

```cpp
modded class MetricZ_Storage
{
#ifdef METRICZ
  protected static ref MetricZ_MetricInt s_MySuperItemToggles = new MetricZ_MetricInt(
    "my_super_item_toggles",
    "Number of ActionToggleMySuperItem completions",
    MetricZ_MetricType.COUNTER
  );

  override static void Init()
  {
    super.Init();
    s_Registry.Insert(s_MySuperItemToggles);
  }
#endif
}
```

Increment from your action:

```cpp
modded class ActionToggleMySuperItem
{
  override void OnFinishProgress(ActionData action_data)
  {
#ifdef METRICZ
    MetricZ_Storage.s_MySuperItemToggles.Inc();
#endif
    super.OnFinishProgress(action_data);
  }
}
```

If needed, attach a static label:

```cpp
#ifdef METRICZ
static void InitMyMetrics()
{
  MetricZ_Storage.s_MySuperItemToggles.MakeLabel("item", "MySuperItem");
}
#endif
```

## Advance

For more advanced scenarios, see the built-in MetricZ implementations:

* for **many dynamic labels on a single metric**, check `infected_mind_state`
  and `infected_by_type` — they show how to create and cache per-label
  metric instances efficiently;
* for **per-object metrics** where each entity needs its own dedicated
  time series, examine `territory_lifetime` and `transport_health`;
* for **a fixed, predefined label set**, follow the pattern used by the
  `food` metric with its `food_type` label.
