local tokenizer = require "tokenizer"

local lexer = {}

function lexer.lex(input)
  local tokenize_coro = tokenizer.tokenize(input)

  local token = nil
  local eof = false

  local function read()
    local succ, tok = coroutine.resume(tokenize_coro)

    if succ then
      if tok then
        if tok.type == "EOF" then
          eof = true
        elseif tok.error then
          panic(tok.error)
        else
          token = tok
        end
      else
        error(succ)
      end
    else
      error(tok)
    end
  end

  local function is(type, value)
    if value == nil then
      return token.type == type
    else
      return token.type == type and token.value == value
    end
  end

  local function skip_newlines()
    while is("CONTROL", "\n") do
      read()
    end
  end

  local function yield(lexeme)
    coroutine.yield(lexeme)
  end

  local function panic(message)
    if message == nil then
      message = "Unexpected token"
    end

    coroutine.yield {
      error = message
    }
  end

  -- Here go parsing methods

  local function parse_expression()
    -- Here we go
  end

  local function parse_restriction()
  end

  local function parse_macro()
  end

  local function parse_function_body()
    local expressions = {}

    while not is("KEYWORD", "end") do
      table.insert(expressions, parse_expression())
    end

    return expressions
  end

  -- Parse single argument declaration.
  local function parse_argument(generic)
    -- Arguments can have aliases, thus keywords are allowed.
    -- For example, `def foo(return type : T)`, which could
    -- be called like this: `foo(return: Int32)`

    if
      (not generic and (is("ID") or is("KEYWORD"))) or
        (generic and is("CONST"))
     then
      local is_keyword = is("KEYWORD")

      local arg = {
        name = token,
        alias = nil,
        type_restriction = nil,
        default_value = nil
      }

      read()

      if (not generic and is("ID")) or (generic and is("CONST")) then
        arg.alias = arg.name
        arg.name = token
        read()
      elseif is_keyword then
        panic("Can't use `" .. arg.name .. "` as argument name")
      end

      if is("CONTROL", ":") then
        read() -- Consume the `:`
        arg.type_restriction = parse_restriction()
      end

      if is("OP", "=") then
        read() -- Consume the `=`
        arg.default_value = parse_expression()
      end

      return arg
    end
  end

  -- Parse function prototype, starting from its name.
  -- For example, `def foo(bar baz : Qux = quux, zax)`.
  local function parse_function_prototype()
    local prototype = {
      name = nil, -- Function name
      op = nil, -- Whether can function be used as an operation
      args = {} -- Function arguments
    }

    if token.type == "ID" then
      prototype.op = false
    elseif token.type == "OP" then
      prototype.op = true -- This is an operation, not a simple method
    else
      panic("Function name expected")
    end

    prototype.name = token
    read() -- Consume the name

    if is("BRACKET", "(") then
      read() -- Consume the "("

      while not is("BRACKET", ")") do
        table.insert(prototype.args, parse_argument(false))

        if is("CONTROL", ",") then
          read() -- Consume the comma
        elseif is("BRACKET", ")") then
          -- Do nothing
        else
          panic() -- Unexpected!
        end
      end

      read() -- Consume the ")"
    end

    return prototype
  end

  -- Parse function.
  local function parse_function(is_abstract)
    local func = {
      kind = "function",
      prototype = parse_function_prototype(),
      body = nil
    }

    if is_abstract then
      return func -- We don't expect body for an abstract function
    end

    skip_newlines()
    func.body = parse_function_body()

    if is("KEYWORD", "end") then
      return func
    else
      error("BUG! Expected end")
    end
  end

  -- Parse a variable; either local or object-scoped.
  local function parse_variable()
    local lexeme = {
      kind = "variable",
      name = nil, -- The variable name
      type = nil, -- The variable type restriction
      value = nil -- The variable value
    }
  end

  -- Parse type "reference" for some expression, e.g. `Cat` or `Foo(T: Bar)`.
  local function parse_typeref()
  end

  -- Parse module, primitive, struct or class, i.e. anything
  -- having attributes and/or methods.
  local function parse_object(type)
    local object = {
      kind = type, -- The object kind (e.g. "struct")
      name = nil, -- The object name
      generics = nil, -- Array of generic arguments
      derives = nil, -- Array of derives (`nil` for namespaces)
      declarations = {} -- Array of sub-namespaces objects
    }

    if is("CONST") then
      object.name = token
      read()
    else
      panic("Expected constant")
    end

    if is("BRACKET", "(") then
      read()

      while not is("BRACKET", ")") do
        table.insert(object.generics, parse_argument(true))

        if is("CONTROL", ",") then
          read() -- Continue to the next argument
        else
          panic()
        end
      end

      read() -- Consume the closing bracket
    end

    -- Shortcut for the first derive.
    -- Can be used for single-type inheritance, e.g. `class Cat < Animal`
    if is("OP", "<") then
      read()
      table.insert(object.derives, parse_typeref())
    end

    local ivars, imethods = true, true

    if type == "primitive" then
      ivars = false
    end

    while true do
      skip_newlines()

      if is("KEYWORD", "end") then
        read()
        break
      elseif is("KEYWORD", "derive") then
        read()
        table.insert(object.derives, parse_typeref())
      else
        table.insert(
          object.declarations,
          parse_declaration(ivars, imethods)
        )
      end
    end

    return object
  end

  -- Parse enum or flag, depending on the *flag* argument.
  -- Enums consist exclusively from their values; they don't have
  -- neither variables nor methods.
  local function parse_enum(is_flag)
    local enum = {
      kind = is_flag and "flag" or "enum",
      name = nil, -- Enum name
      type = nil, -- Enum type (e.g. UInt8)
      values = {} -- Array of enum values
    }

    if is("CONST") then
      enum.name = token
      read()
    else
      panic("Expected constant")
    end

    -- Enums are allowed to have explicit type, e.g. `enum Foo : UInt32`
    if is("CONTROL", ":") then
      read() -- Consume the type restriction character

      if is("CONST") then
        enum.type = token
        read() -- Consume the type constant
      else
        panic("Expected explicit enum type")
      end
    end

    while is("CONST") do
      local value = {
        name = token
      }

      read()

      if is("OP", "=") then
        read()

        if is("NUMERIC") then
          value.value = token
          read()
        else
          panic("Expected numeric value")
        end
      end

      table.insert(enum.values, value)
      skip_newlines()
    end

    if is("KEYWORD", "end") then
      read()
    else
      panic("Expected end")
    end

    return enum
  end

  -- Parse namespace. Namespaces don't have neither instance variables
  -- not instance methods. They also can't derive.
  local function parse_namespace()
    if is("CONST") then
      local namespace = {
        kind = "namespace",
        name = token,
        declarations = {}
      }

      read() -- Consume the CONST
      skip_newlines()

      -- We expect that the Tokenizer takes care of the block completeness,
      -- thus we're guaranteed to have the `end` keyword met
      while not is("KEYWORD", "end") do
        table.insert(
          namespace.declarations,
          parse_declaration(false, false)
        )
      end

      read() -- Consume the `end` keyword
      return namespace
    else
      panic("Expected constant")
    end
  end

  -- Parse a declaration of something.
  -- *ivars* and *imethods* decide if instance variables and methods are allowed.
  local function parse_declaration(ivars, imethods)
    if not is("KEYWORD") then
      panic(token, "Declaration expected")
    end

    local modifiers = {}

    -- Returns *n*th modifier those value is in *keys*.
    local function nth_modifier(n, keys)
      return table.nth(
        modifiers,
        n,
        function(k, v)
          if table.includes(keys, k) then
            return true
          end
        end
      )
    end

    -- Find a bad token in the list of modifiers,
    -- and panic if such. Otherwise do nothing.
    local function ensure_modifiers(allowed)
      local bad =
        table.find(
        modifiers,
        function(key)
          if not table.includes(allowed, key) then
            return true
          end
        end
      )

      if bad then
        panic(
          "Invalid modifier " .. bad.value .. " for " .. token.value,
          bad
        )
      end
    end

    -- Iterate through tokens while they're in the list of known modifiers
    while (token.type == "KEYWORD" and
      table.includes(
        {
          "abstract",
          --
          "private",
          "protected",
          --
          "static",
          --
          "atomic",
          "volatile",
          --
          "var",
          "const",
          --
          "getter",
          "setter",
          "property"
        },
        token.value
      )) do
      if modifiers[token.value] then
        panic(token, "Duplicate modifier")
      else
        modifiers[token.value] = token
      end

      read()
    end

    -- If the next token is not a modifier, but still a keyword,
    -- then we further check this keyword. Otherwise, if the token
    -- is identifier, it must be a variable then.

    if token.type == "KEYWORD" then
      if
        -- Different lexemes
        table.includes(
          {
            "namespace",
            "module",
            "primitive",
            "struct",
            "class",
            "enum",
            "flag",
            "macro"
          },
          token.value
        )
       then
        if table.includes({"struct", "class"}, token.value) then
          -- Only structs and classes can be abstract
          ensure_modifiers({"abstract", "private", "protected"})
        else
          ensure_modifiers({"private", "protected"})
        end

        if modifiers.private and modifiers.protected then
          panic(
            token.value:capitalize() ..
              " can be either private or protected",
            nth_modifier(2, {"private", "protected"})
          )
        end

        local lexeme = nil

        if token.value == "namespace" then
          lexeme = parse_namespace()
        elseif token.value == "enum" or token.value == "flag" then
          lexeme = parse_enum(token.value == "flag")
        elseif token.value == "macro" then
          lexeme = parse_macro()
        else
          lexeme = parse_object(token.value)
        end

        lexeme.modifiers = modifiers
        return lexeme
      elseif -- Functions
        token.value == "def" or token.value == "redef" then
        ensure_modifiers(
          {
            "abstract",
            "static",
            --
            "private",
            "protected",
            --
            "atomic",
            "const"
          }
        )

        if modifiers.private and modifiers.protected then
          panic(
            "Function can be either private or protected",
            nth_modifier(2, {"private", "protected"})
          )
        end

        if (not modifiers.static) and (not imethods) then
          panic("Instance methods aren't allowed here")
        end

        -- Abstract functions don't have body
        local func = parse_function(modifiers.abstract)

        func.modifiers = modifiers
        return func
      else
        panic(
          "Expected namespace, module, primitive, \
          struct, class, enum, flag, def or macro"
        )
      end
    elseif token.type == "ID" then
      local found = false

      for _, mod in ipairs(
        {"var", "getter", "setter", "property", "const"}
      ) do
        if modifiers[mod] then
          if not found then
            found = true
          else
            panic(
              "Variable declaration can be either \
              var, getter, setter, property or const"
            )
          end
        end
      end

      if not found then
        panic()
      end

      -- -- If *accessors* is falsey, then getter, setter and property
      -- -- are disallowed
      -- if
      --   (not accessors) and
      --     (modifiers.getter or modifiers.setter or modifiers.property)
      --  then
      --   panic(
      --     "Variable accessors are not allowed here",
      --     nth_modifier(1, {"getter", "setter", "property"})
      --   )
      -- end

      if (not ivars) and not (modifiers.static or modifiers.const) then
        panic("Instance variables are not allowed here")
      end

      local variable = parse_variable()

      if modifiers.const and not variable.value then
        panic("Constants must have a default value", variable.id)
      end

      if modifiers.static and not variable.value then
        panic("Static variables must have a default value", variable.id)
      end

      variable.modifiers = modifiers
      return variable
    else
      panic("Expected keyword or identifier")
    end
  end

  local function parse()
    local lexeme = parse_declaration()
    yield(lexeme)
  end

  return coroutine.create(
    function()
      read()

      while not eof do
        parse()
      end
    end
  )
end

return lexer
