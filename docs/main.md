# Note
The following document introduces basic C# techniques used in InfiCell; the more complicated C++ documentation can be found at the Harpoon repo [documentation](https://github.com/Nielsbishere/Harpoon/tree/master/docs/main.md).
# Introduction

I don't advise to use code injection unless you are 100% aware of what you're doing, I think code injection can be really dangerous and should only be used when completely aware of the impact. I'm applying these techniques so that I can build a mod loader for Unity games, so others can add their content to games. When making a mod loader, be aware of the EULA surrounding it, so you don't get sued.

The objective of this document is security and modding research; as such, it mentions ways of protecting and implementing the injection. However, not everything can be fool-proof.

**NOTE: I am not responsible for anything caused
by readers of this document, please solely read this for a protection manual or
use it responsibly for good; not evil.** 

# C# reflection

## Definition

Reflection is a tool available in most modern (high level) languages. This tool allows you to query all information about classes that you need, allowing you to inspect any object very easily. In C++, this is not available, this is because C++ compiles directly to assembly, which in turn is translated to machine code. This assembly doesn't contain anything else than simple instructions; so, the idea of classes doesn't actually exist, it's just data types at offsets in buffers. (Most) functions get inlined (especially in Release mode), making it harder to reverse engineer them. The combination of this means that C++ code is impossible to use reflection on, as it's not built in. However, other languages (like C#, Java, etc.) have this tool available without any additional work. This is because both languages store reflection data low level (in the assembly), meaning that a field is not just an offset in a buffer, it also has a name. Java stores it in the class bytecode and C# stores it in the DLL assembly.

However, general reflection shouldn't be a problem (being able to just read and NOT write), the security issue arises when you're allowed to inspect and modify protected/private variables. These variables were made private/protected because they aren't meant to be exposed, but reflection allows you to do so anyways.

This opens up the opportunity to insert your own information into already existing lists, maps or call constructors/functions/variables that shouldn't be exposed.

## Use

Mostly it is used by users of an application to get around protections put in place against them, or just general bad design (since we're working with Unity). For modders, this is generally the case, especially if working with Indie games; which generally don't have a high coding standard. They can then find/access private/protected fields/methods/constructors/properties (I call them; class objects) and insert their own information where required.

## Execution (Pseudo-code)

An example of accessing a property;

 get type (class identifier)  
 get property (NonPublic | Instance)  
 SetValue / GetValue  

As you can see, it is very easy to manipulate data as long as you already have the assembly referenced. You can obtain the type runtime (x.GetType()) or compile-time (typeof(T)). Then, you can input the name of the property and the binding flags (or you can request all properties and loop through them). 

The binding flags describe how the class objects' modifiers are specified. This means that 'public static float var = …' would require no binding flags, since it is public static. private would require the binding flag 'NonPublic' as well as the protected keyword, while Instance is used when the static modifier is not present. If the Instance binding flag is specified, you require to input an instance as the first parameter for set/get value instead of inputting null.

## Execution (C#)

The following example is taken from ShipLoader.API (written by me):

```C#
Type t = typeof(ItemInstance_Recipe);
```

Here we obtain the class info about the ItemInstance_Recipe class. This class contains
the info we need to modify.

```c#
PropertyInfo p = t.GetProperty("Category", BindingFlags.NonPublic | BindingFlags.Instance);
```

We then get the property "Category", which is a private instance property.

```c#
p.SetValue(baseItem.settings_recipe, (CraftingCategory) category, null);
```

We then input the ItemInstance_Recipe instance as the first parameter and the value of the Category as our second variable. This would work the same as "baseItem.settings_recipe.Category = (CraftingCategory) category;", but that wouldn't compile, because Category is protected.

```c#
Type t = typeof(Item_Base);
FieldInfo f = t.GetField("type", BindingFlags.NonPublic | BindingFlags.Instance);
```

The example above uses Item_Base's type field that is also protected. We need to set it, which is almost the same as setting a property, except that there is no third argument.

```C#
f.SetValue(baseItem, (ItemType) use);
```

As shown above, it is almost the same as using a
property.

```C#
Type t = typeof(RaftMod);
MethodInfo m = t.GetMethod("RemoveRecipe", BindingFlags.Instance | BindingFlags.NonPublic);
```

Above we acquire the 'private RemoveRecipe' function, which we can then invoke.

```c#
m.Invoke(recipe.owner, new object[]{ recipe });
```

The first argument is the instance (as always) and the second is the parameters to the function; an object[].

```C#
Mod m = (Mod) Activator.CreateInstance(t);
```

You can create an instance of a Type (t) by using the Activator class, allowing you to access private constructors.

All of these methods now allow you to remove almost any protection in a C# API or game, allowing you to even load libraries yourself and load data from them.

## Prevention

The prevention of C# reflection is impossible if you use things like Unity; since they require this reflection data. However, you can obfuscate the code, making it harder for people to understand what is going on in your code. Unfortunately, C# is incredibly easy to reverse engineer (using something like dnSpy), so even obfuscation won't help you if people really want to change your code.

# Code replacement

## Definition

With our previous chapters, you will only be able to use the existing code. You can run your C# code and/or C++ code alongside the program, but you'll never be able to change existing code. Harmony changes this; it is a library that allows the modification of code by injecting code before/after or at the current function call. 

## Use

You use code replacement because there is something you want the game code to do (or not to do), that is impossible without intervention. Most of the time, this is the case when there's no more backdoors left via reflection (like when there's a buggy line of code that prevents you from adding content to the game). However, in harmful software it could also be used to hook into existing functions and prevent them from working and/or validating various things.

## Execution (Pseudo-code)

### Replacing static functions

create harmony instance (static constructor)  
 find static method to replace  
 ask harmony to put it as prefix or suffix  

### Replacing instance functions

create harmony instance (static constructor)  
 find instance method to replace  
 create function that has T __instance as first param and ref T2 result (if the function doesn't return void)  
 ask harmony to put it as prefix or suffix  

## Execution (C#)

### Replacing static functions

```C#
HarmonyInstance harmony = HarmonyInstance.Create("shiploader.api.harmony");
```

Creating an instance is very easy, you just need an identifier for what you're going to use it for (shiploader.api for example).

Next; you need to get the MethodInfo using C# reflection that you want to replace.
 Afterwards, you can create a 'HarmonyMethod' object using the MethodInfo of the function you want to put as prefix or suffix. Which you have to input into harmony.Patch.

```C#
harmony.Patch(targetMethod, prefixMethod, null);
```

targetMethod is a MethodInfo of the function we want to replace. prefixMethod is the HarmonyMethod that we want to run before the function and the third argument is the suffixMethod (HarmonyMethod). If you want to exit out of the function; you can change the return type to bool and return false; this will prevent other code from running (only available for prefix).

### Replacing instance functions

We use the same way as before; the only difference is that we won't use member functions, but use the `__instance` parameter as the first argument and then the function's arguments. This means that the function 'string Test(string a)' (in Test2 class) would be represented as `public static void MyPrefix(Test2 __instance, string a, ref string __result)`

This means that you can now access `this` via `__instance` and set the result by using the `__result` variable. If you want to exit out of the function; you can change the return type to bool and return false; this will prevent other code from running (only available for prefix methods).

## Prevention

There is not much you can do against Harmony; except preventing the library from loading. This is pretty hard though, as Harmony is open source and that means someone can modify it in a way so it won't be detected anymore. 

## Source

Tutorial on using [Harmony](https://github.com/pardeike/Harmony/wiki) at Github ([roxxploxx](https://github.com/roxxploxx/RimWorldModGuide/wiki/SHORTTUTORIAL%3A-Harmony)).
