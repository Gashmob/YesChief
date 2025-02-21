# Specifications

Description of intended functionalities of YesChief! for release `1.0.0`.

There will be 2 distinct parts : options and commands (even if commands can have options).

## Options

Each option have a name, an optional short name, a description and a type. If the type is omitted it default to
boolean (present or not).

```c++
// All begins with the CLI object, it represents your program
auto cli = CLI("MyProgram", "Short description of MyProgram");

// General configuration
cli.setLongDescription("Long description of MyProgram");

// You can then add some options
// The parameters are name and description
// For the name you can set only a long name, or set also a short name by doing "<long>,<short>"
cli.addOption("version,V", "Show version and exit")
   // Calls can be chained
   // A third parameter is available to set some options
   // A type can also be specified
   .addOption<std::string>("name", "Name of the person", { .required: true })
   // If the type is a vector then the option can be repeated or set with a comma separated values string
   .addOption<std::vector<std::string>>("numbers", "Numbers to print");
   
// Options can be grouped
cli.addGroup("Troubleshouting")
   .addOption("verbose", "Display more informations");
   
// You can ask one option or more to be parsed as a positional argument
// This means that user doesn't need to use the option name and directly pass the value
cli.parsePositional("name");

// You can then run the program
cli.run(argc, argv)
    .and_then([&cli](const CLIRestuls &results) {
        results.get("version").and_then([]() {
            std::cout << "1.0.2\n";
        });
        results.get("name").and_then([](const std::string &name) {
            std::cout << "Hello " << name << "!\n";
        });
    
        return 0;
    })
    .or_else([]() {
        std::cout << "Something went wrong during cli run\n";
        return 1;
    });
```

## Commands

Each command have a name, a short description and an optional long description

```c++
// All begins with the CLI object, it represents your program
auto cli = CLI("MyProgram", "Short description of MyProgram");

// General configuration
cli.setLongDescription("Long description of MyProgram");

// You can then define commands with class implementation of `Command`
class MyCommand : public Command {
  public:
    MyCommand();
    
    auto setup(CLI &cli) -> void;
    
    auto run(const CLIResult &results) -> int;
};

MyCommand::MyCommand() : Command(
    "my-command",
    "Short description",
    "Loooong description",
)

MyCommand::setup(CLI &cli) -> void {
    // In setup you can add options or even sub-commands to your command
    cli.addOption<std::vector<int>>("numbers", "Numbers to add");
    cli.parsePositional("numbers")
}

MyCommand::run(const CLIResults &results) -> int {
    // Here you can run your command with the results of argv parsing
    results.get("numbers")
        .and_then([](const std::vector<int> &numbers) {
            std::cout << std::reduce(numbers.begin(), numbers.end(), 0, std::plus<int>()) << "\n";
        })
        .or_else([]() {
            std::cout << "0\n";
        });
        
    return 0;
}

// Then you can add it to the program
cli.addCommand(MyCommand());

// No need to go into the and_then, the command will run automagicly
cli.run(argc, argv).or_else([]() {
    std::cout << "Something went wrong!\n";
    return 1;
});
```

## Some details

- You cannot have commands and options at the same level of execution
