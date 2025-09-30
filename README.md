# Stush
---

stush is a stupid shell. It was created as an educational project and you certainly don't want to run it as you login shell.
Compliance with any existing standards is (at least for now) coincidental.

## Features
### Implemented:
- [x] Line editing (my own library)
- [x] Running scripts from files
- [x] Running external commands
- [x] Running shell builtins
- [x] Variable expansion (both shell and environment)
- [x] Tilde expansion
- [x] Glob expansion
- [x] Pipelines (| and |&)
- [x] Command lists (|| and &&)
### Not (yet) implemented:
- [ ] Line editing (using GNU readline or similar)
- [ ] Shell configuration
- [ ] Prompt customization
- [ ] Command history
- [ ] Multi-line commands
- [ ] Brace expansion
- [ ] Parameter expansion
- [ ] Redirections and heredocs
- [ ] Command substitution
- [ ] Any kind of scripting language
- [ ] Job control

## Usage
Currently there are three ways to use stush:
1. Interactive mode (default): simply run the stush executable.
2. Running a command from command line args: `stush -c 'command'`
3. Running script files:
    * `stush <filename>` - run file as stush script
    * Create a script with a shebang pointing to stush's location and run it
Type `help` in interactive mode to get info on all shell builtins.

## Building
`cmake -S . -B build && cmake --build build`
Dependencies:
* CMake (build)
* GoogleTest (testing)

## Platforms
Stush was developed on Linux and currently supports only it.
