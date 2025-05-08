# GitHub Copilot Instructions for Silver Bucket Mudlib

## Codebase Overview
This is a Silver Bucket Mudlib codebase for a LPC-based MUD (Multi-User Dungeon) using the LDMud driver. The codebase is organized into modules and follows specific LPC coding patterns and conventions.

## Language and Paradigm
- This codebase is written in LPC, an object-oriented language used primarily for MUD development
- The code follows an object-oriented approach with inheritance and closures
- Files generally use `.c` extension for implementation files and `.h` for header/include files
- Heads up: The codebase is converted from an old LPMud library and there still exists quite a few unconverted files.

## Code Structure
- `/std/` - Standard library objects (rooms, NPCs, items, etc.)
- `/lib/` - Core library functions
- `/secure/` - Security-critical code including master object and simul_efuns
- `/inc/` - Include files with constants, macros and shared definitions
- `/obj/` - Object definitions
- `/d/` - Domain/area definitions
- `/players/` - Player data
- `/w/` - Wizard's presonal workspaces

## Type Safety and Strict Types
- Core libraries (`/std/` and `/lib/`) use `#pragma strict_types` for type safety
- Domain and wizard workspaces (`/d/` and `/w/`) typically use looser typing
- When using `#pragma strict_types`, remote function calls must be explicitly type cast:
  ```
  // Without strict types:
  int value = obj->get_value();
  
  // With strict types, must explicitly cast the return value:
  int value = ({int}) obj->get_value();
  
  // For arrays, mappings or other complex types:
  string *names = ({string *}) obj->get_names();
  mapping data = ({mapping}) obj->get_data();
  ```
- This type casting is especially important in library code that uses strict types

## Naming Conventions
- Function names: snake_case (e.g., `reset_auto_objects`, `add_npc`)
- Variable names: snake_case (e.g., `room_objects`, `clone_count`)
- Try to keep the names descriptive with as few abbreviations as possible. There is still a lot of legacy code with generic names that should be changed when they are encountered.
- Constants: ALL_CAPS (typically defined in include files)

## Common Coding Patterns
- Object cloning with `clone_object()`
- Using closures with `#'function_name`, or if not possible, (:  :) syntax
- Mapping data structures for object management
- Type checking with `objectp()`, `stringp()`, `mappingp()`, etc.
- Reset mechanisms for room objects and NPCs

## Error Handling
- Function validation of argument types (e.g., checking if arguments are strings, objects, etc.)
- Appropriate returns or early exits when invalid conditions are detected

## Legacy Code Issues to Modernize

- **Property Values and Closures**: Modernize closure and function reference syntax:
  ```
  // Legacy code - DO NOT USE:
  add_prop(OBJ_I_VALUE, &query_value());
  
  // Simple function reference - PREFERRED:
  add_prop(OBJ_I_VALUE, #'query_value);

  // Alternative syntax when #' isn't possible:
  add_prop(OBJ_I_VALUE, (: query_value :));
  
  // For filtering with a method call:
  // Legacy: 
  filter(object_list, &->func())
  // Modern:
  filter(object_list, (: ({int}) $1->func() :))
  
  // For map with extra arguments:
  // Legacy:
  map(some_list, &func(, arg2))
  // Modern:
  map(some_list, #'func, arg2)
  
  // When not the first argument is variable:
  map(some_list, (: func($1, arg2) :))  // $1 is first
  map(some_list, (: func(arg1, $1) :))  // $1 is second
  ```

- **set_alarm to call_out**: The `set_alarm()` function is deprecated and should be replaced with `call_out()`:
  ```
  // Legacy code - DO NOT USE:
  set_alarm(0.1, 0.0, &remove_broken(0));
  
  // Modern approach:
  call_out(#'remove_broken, 0, 0);  // function, delay, argument
  
  // With multiple arguments:
  set_alarm(2.0, 0.0, &tell_room(room, message, exclude));  // Legacy
  call_out(#'tell_room, 2, room, message, exclude);         // Modern
  
  // Managing alarm IDs (important difference):
  
  // Legacy code with alarm IDs:
  int alarm_id = set_alarm(10.0, 0.0, &check_status(target));
  // Later, to remove it:
  remove_alarm(alarm_id);
  
  // Modern approach uses function reference instead of IDs:
  call_out(#'check_status, 10, target);
  // Later, to remove it:
  remove_call_out(#'check_status);  // Removes the next pending call_out to check_status

  // To find the time remaining, commonly used to find out if call out is active:
  int time_left = find_call_out(#'check_status);
  if (find_call_out(#'check_status)) { ... }
  
  // For repeating alarms (periodic tasks):
  
  // Legacy code with repeat delay:
  set_alarm(5.0, 60.0, &periodic_check());  // First after 5s, then every 60s
  
  // Modern approach - self-scheduling in the function:
  void periodic_check() {
      // Do the periodic task
      
      // Schedule the next call
      call_out(#'periodic_check, 60);
  }
  ```

- **Member Function**: The `member()` function parameter order has changed from `member(item, array)` to `member(array, item)`. Always ensure the array is the first parameter:
  ```
  // Legacy code - DO NOT USE:
  if (member(item, my_array) != -1)
  
  // Modern approach:
  if (member(my_array, item) != -1)
  
  // Alternative when index isn't important:
  if (item in my_array)
  ```

- **Wildmatch Function**: The `wildmatch()` function no longer exists and should be replaced with equivalent functionality using regular expressions or other pattern matching approaches. See documentation in `/doc/cd_doc/man/efun/wildmatch` for the original behavior to replicate. Replace with `regexp()` where appropriate.

## When Modifying Code
- Maintain consistent style with the existing codebase. Legacy code may need some modernization though.
- Function declarations should have the type and access modifiers on the same line as the function name. Only put arguments on separate lines if the line gets too long:
  ```
  // Preferred style (all on one line when possible)
  public string short(object for_obj)
  
  // For longer function declarations, arguments can go on separate lines
  public mapping query_complex_data(
      int type_id, 
      string category,
      object requester)
  ```
- Include appropriate comments for functions using the established format:
  ```
  /* 
   * Function Name: function_name
   * Description  : What the function does
   * Arguments    : arg1 - description
   *                arg2 - description
   * Returns      : What the function returns
   */
  ```
- Use proper LPC syntax and MUD-specific functions
- Consider object persistence and reset mechanisms
- Test changes in a development environment before deploying

## Docker Environment
- The codebase runs in a Docker environment with the LDMud driver
- Configuration is handled via docker-compose.yml
- Server runs on port 4040
