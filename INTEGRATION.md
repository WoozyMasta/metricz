# Using MetricZ in Your Own Mods

MetricZ can be used as a metrics library inside other mods.
Dependency is optional: wrap all calls in `#ifdef METRICZ`.

## Compile-time guard

To avoid hard dependency errors when MetricZ is not installed,
wrap your integration code:

```cpp
#ifdef SERVER
#ifdef METRICZ
// Code using MetricZ classes
#endif
#endif
```

If MetricZ is not loaded, the block is skipped.

## Core metric types

The mod provides wrapper classes for Prometheus metric types.

> [!NOTE]  
> Metric names are provided *without* the `dayz_metricz_` prefix -
> it is added automatically.

```cpp
class MetricZ_MetricInt;   // Integer counters and gauges
class MetricZ_MetricFloat; // Floating point gauges
class MetricZ_MetricBase;  // Base class
enum MetricZ_MetricType { GAUGE, COUNTER }
```

## Defining and Registering Metrics

The standard way to add metrics is to extend the `MetricZ_Storage` class.
This ensures your metrics are initialized, stored in the global registry,
and automatically exported to all active sinks (File, HTTP, etc.).

> [!NOTE]  
> Do not use `protected` visibility for the static metric variable if
> you intend to access it from other classes (e.g., `PlayerBase`).  
> Use `static ref` (package/public visibility).

### Example: Counting Player Jumps

Define and Register:*

```cpp
// 4_World/MyMod/MetricZ_Integration.c

modded class MetricZ_Storage
{
#ifdef METRICZ
    // Define the metric instance (static, accessible)
    static ref MetricZ_MetricInt s_MyMod_PlayerJumps = new MetricZ_MetricInt(
        "player_jumps",                 // Name (will be dayz_metricz_player_jumps_total)
        "Total player jump starts",     // Help string
        MetricZ_MetricType.COUNTER      // Type
    );

    override static void Init()
    {
        super.Init(); // Run MetricZ built-in initialization first

        // Register your metric in the global registry
        // This ensures it gets written to file/sent via HTTP automatically
        s_Registry.Insert(s_MyMod_PlayerJumps);
    }
#endif
}
```

Instrument the Gameplay Code:

```cpp
// 4_World/MyMod/PlayerBase.c

modded class PlayerBase
{
    override void OnJumpStart()
    {
#ifdef METRICZ
        // Check if storage is initialized to avoid null pointer access during early startup
        if (MetricZ_Storage.IsInitialized())
            MetricZ_Storage.s_MyMod_PlayerJumps.Inc();
#endif

        super.OnJumpStart();
    }
}
```

Result:

```PromQL
# HELP dayz_metricz_player_jumps_total Total player jump starts
# TYPE dayz_metricz_player_jumps_total counter
dayz_metricz_player_jumps_total{world="chernarusplus",host="dz01",instance_id="1"} 42
```

## Advanced: Custom Collector Modules

If your mod collects complex data, requires high performance,
or you want to keep your logic isolated from the global storage,
use the **Collector API**.

This approach runs your metric collection in its own separate
server frame (tick), preventing lag spikes during the export cycle.

### Create a Collector Class

Inherit from `MetricZ_CollectorBase` and override the `Flush` method.

```cpp
// 5_Mission/MyMod/MetricZ_MyCollector.c

#ifdef METRICZ
class MetricZ_Collector_MyMod : MetricZ_CollectorBase
{
    // Unique name for logging and internal profiling
    override string GetName()
    {
        return "my_super_mod";
    }

    // Logic to write metrics to the sink
    override void Flush(MetricZ_SinkBase sink)
    {
        // Example: Iterate over internal data
        // metric.Set(10);
        // metric.Flush(sink);
    }
}
#endif
```

### Register in MissionServer

Register your collector inside `MissionServer::OnInit`.

> [!WARNING]  
> Always call `Register` **after** `super.OnInit()`.
> Calling it earlier will fail because MetricZ is not initialized yet.

```cpp
// 5_Mission/MyMod/MissionServer.c

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();

#ifdef METRICZ
        MetricZ_Exporter.Register(new MetricZ_Collector_MyMod());
#endif
    }
}
```

## Labels

Labels allow you to add dimensions to your metrics.
Base labels (`world`, `host`, `instance_id`) are added
automatically to all metrics.

### Static Labels

If a metric always has a fixed set of extra labels, define them during initialization.

```cpp
#ifdef METRICZ
static ref MetricZ_MetricInt s_MyItemUses = new MetricZ_MetricInt(
    "my_item_uses",
    "Specific item use count",
    MetricZ_MetricType.COUNTER
);

// Call this inside MetricZ_Storage::Init or immediately after creation
void SetupMyLabels()
{
    // Single label
    s_MyItemUses.MakeLabel("item", "MySuperItem");
    
    // OR Multiple labels
    // map<string, string> labels = new map<string, string>();
    // labels.Insert("tier", "5");
    // labels.Insert("rarity", "legendary");
    // s_MyItemUses.MakeLabels(labels);
}
#endif
```

### Dynamic Labels (Advanced)

If you need high-cardinality metrics
(e.g., counting interactions per item type),
you cannot use a single `MetricZ_MetricInt`
instance because it holds one value.

You must implement a management class similar to
`MetricZ_WeaponStats` or `MetricZ_ZombieStats` that:

1. Maintains a `map<string, int>` of counters.
1. Maintains a `map<string, string>` of pre-formatted labels strings.
1. Iterates over the map in a `Flush()` method and writes to the sink.

See `3_Game/MetricZ/Stats/RPC.c` or
`4_World/MetricZ/Entities/Weapons/HitStats.c`
in the source code for reference implementations.*

## Entity Label Overrides

MetricZ uses heuristics to determine readable names for
Zombies, Animals, and Weapons.
If your mod adds custom entities that are not correctly categorized by the
default logic, you can override the helper methods in your entity classes.

### Custom Zombie Name

Default logic checks `aiAgentTemplate`.
If your zombie uses a custom template or generic class, override:

```cpp
class MySuperZombie : ZombieBase
{
#ifdef METRICZ
    override string MetricZ_GetLabelTypeName()
    {
        return "my_super_zombie"; // Resulting label: type="my_super_zombie"
    }
#endif
}
```

### Custom Animal Name

Default logic looks at Skinning config (Steaks/Pelt).

```cpp
class MySuperAnimal : AnimalBase
{
#ifdef METRICZ
    override string MetricZ_GetLabelTypeName()
    {
        return "my_super_animal";
    }
#endif
}
```

### Custom Weapon Name

Default logic strips common suffixes (color, camo, damage state).

```cpp
class MySuperWeapon : Weapon_Base
{
#ifdef METRICZ
    override string MetricZ_GetLabelTypeName()
    {
        return "my_super_rifle";
    }
#endif
}
```

### Custom Item Name

Applies to any `ItemBase`.

```cpp
class MyModdedItem : ItemBase
{
#ifdef METRICZ
    override string MetricZ_GetLabelTypeName()
    {
        return "my_clean_item_name";
    }
#endif
}
```

### Custom Food Category

MetricZ groups food into static enums (Fruit, Meat, Canned, etc.)
to keep series cardinality low.
If your item isn't categorized correctly:

```cpp
class MySuperJarFood : Edible_Base
{
#ifdef METRICZ
    override MetricZ_FoodTypes MetricZ_GetFoodType()
    {
        // Must return a value from MetricZ_FoodTypes enum
        // See 4_World/MetricZ/Entities/Items/Edible_Base.c
        return MetricZ_FoodTypes.JAR;
    }
#endif
}
```

## Best Practices

1. **Cardinality:**
  Do not create labels with user input(player names, chat messages) or
  unique IDs (GUIDs) unless absolutely necessary.
  High cardinality slows down Prometheus.
    * *Bad:* `my_metric{player_name="Survivor123"}` (Unlimited variations).
    * *Good:* `my_metric{player_type="police"}` (Fixed variations).
1. **Performance:**
  The export loop runs every 15s (default).
  Keep your metric updates lightweight (e.g., increment integer counters).
  Do not perform heavy calculations inside the `MetricZ_Storage` getters.
1. **Sink Agnostic:**
  By registering in `MetricZ_Storage.s_Registry`, your metrics automatically
  support both File Export and HTTP Push. You do not need to handle IO manually.
