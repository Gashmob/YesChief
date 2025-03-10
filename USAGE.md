# Usage documentation

> [!tip]
> This library needs at least c++23 features.

The main class of the library, the one that you will use no matter your usage of this library: `CLI`. This class holds
and manage options and commands of your program.

Its use is in 3 steps:

1. Instantiate it
2. Add options or commands ('xor', both or not allowed)
3. Run it against your program arguments

In code, it translates to:

```cpp
#include <yeschief.h>

auto main(int argc, char **argv) -> int {
    // 1. Instantiation
    auto cli = yeschief::CLI("MyProgram", "Short description of MyProgram");
    
    // 2. Add options
    cli.addOption("help", "Show this help message");
    // We'll see more later
    
    // 3. Run it
    const auto result = cli.run(argc, argv);
    if (! result.has_value()) {
        return 1;
    } else {
        const auto result_values = result.value();
        if (result_values.get("help").has_value()) {
            cli.help();
        }
    }
}
```

`CLI::run` returns a `std::expected<yeschief::CLIResults, yeschief::Fault>`:

- In case all was done well, `CLIResults` is a wrapper for a map of your options. With `get` you access to given value
  for each option. As an option has not always a value the return type is `std::optional<std::any>`.
- In case user has given bad options a `Fault` is returned with a detailed message and the type of Fault (this way you
  can switch on it).

CLI has a special method `yeschief::CLI::help` displaying a help message built from options (or commands).

## Options

Add an option to `CLI` is pretty easy: `cli.addOption("name", "description")` adds an option named 'name' with
'description' as the description.

But the method `addOption` can be used in a more complicated way.

- You can change the type of your option with the template parameter `addOption<int>(...)`. By default, it is bool and
  int, float, double, std::string and std::vector of them are allowed. This type will be the one returned (behind any)
  by `CLIResults::get` and constraint what user can give as a value for the option.
- First parameter of the method correspond to the long name of the option (`--name`). If your option have also a short
  name (`-n`) then it is given in the same parameter but separated with a comma: `name,n`. It should always be long then
  short name and short name can only be single letter.
- The method can get a third parameter for advanced configuration. This parameter is a structure
  (yeschief::OptionConfiguration) with default values. See documentation of it for detailed explanations.

Options can be grouped into option groups. For that you first declare a group with `yeschief::CLI::addGroup`
with a name, and then you can add as many options as you want to this group. Groups have a meaning only in help message.

Finally, your options can be parsed as positional arguments if you need. You can specify which one in which order with
`yeschief::CLI::parsePositional`.

## Commands

All begins with abstract class `yeschief::Command`. To create a command you first need to implement this class into your
own one (one per command ideally). Then you can add these commands to your cli simply with `yeschief::CLI::addCommand`.

There is 2 method you have to implement: `getName` and `run`. The first one just give the name of your command, the one
user will need to use to invoke the command. The second is executed by CLI when user invoke the command, it takes one
argument which is the result of the inner CLI of the command. Because yes, each command have its own CLI which can be
configured inside method `setup`. In it, you can configure the CLI like you do for all others, this way each command can
be seen as a sub-program.

An additional method `getDescription` is available if you want to display a description for your command in the help
message.

A helper command (`yeschief::HelpCommand`) is already defined for the help command. You can take inspiration from it for
all your commands.

> [!tip]
> If anything is unclear, if you have remarks, you are welcome to open an issue on the GitHub repository.
