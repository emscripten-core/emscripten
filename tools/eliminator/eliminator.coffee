###
  A script to eliminate redundant variables common in Emscripted code.

  A variable is eliminateable if it matches a leaf of this condition tree:

  Single-def
    Single-use
      Uses only simple nodes
        Uses only local, single-def names
          *
        Uses non-local or non-single-def names
          No flow-controlling statements between def and use
            No references to any deps between def and use
              *
    Multi-use
      Uses only simple nodes
        Uses only single-def names
          *

  TODO(max99x): Eliminate single-def undefined-initialized vars with no uses
                between declaration and definition.
###

# Imports.
uglify = require 'uglify-js'
fs = require 'fs'

# Node types which can be evaluated without side effects.
SIMPLE_NODES =
  name: true
  num: true
  string: true
  binary: true
  sub: true
  string: true

# Maximum number of uses to consider a variable not worth eliminating.
MAX_USES = 3

# The UglifyJs code generator settings to use.
GEN_OPTIONS =
  ascii_only: true
  beautify: true
  indent_level: 2

# Traverses a JavaScript syntax tree rooted at the given node calling the given
# callback for each node.
#   @arg node: The root of the AST.
#   @arg callback: The callback to call for each node. This will be called with
#     the node as the first argument and its type as the second. If a
#     non-undefined value is returned, it replaces the passed node in the tree.
#     If false is returned, the traversal is stopped.
#   @returns: If the root node was replaced, the new root node. Otherwise
#     undefined.
traverse = (node, callback) ->
  type = node[0]
  if type
    result = callback node, type
    if result? then return result

  for subnode, index in node
    if typeof subnode is 'object' and subnode?.length
      # NOTE: For-in nodes have unspecified var mutations. Leave them alone.
      if type == 'for-in' and subnode?[0] == 'var' then continue
      subresult = traverse subnode, callback
      if subresult is false
        return false
      else if subresult?
        node[index] = subresult
  return undefined

# A class for eliminating redundant variables from JavaScript. Give it an AST
# function/defun node and call run() to apply the optimization (in-place).
class Eliminator
  constructor: (func) ->
    # The statements of the function to analyze.
    @body = func[3]
    # Whether the identifier is never modified after initialization.
    @isSingleDef = {}
    # How many times the identifier is used.
    @useCount = {}
    # Whether the initial value of a single-def identifier uses only nodes
    # evaluating which has no side effects.
    @usesOnlySimpleNodes = {}
    # Whether the initial value of a single-def identifier uses only other
    # local single-def identifiers and/or literals.
    @usesOnlySingleDefs = {}
    # Whether the dependencies of the single-def identifier may be mutated
    # within its live range.
    @depsMutatedInLiveRange = {}
    # Maps a given single-def variable to the AST expression of its initial value.
    @initialValue = {}
    # Maps identifiers to single-def variables which reference it in their
    # initial value.
    @dependsOn = {}

  # Runs the eliminator on a given function body updating the AST in-place.
  #   @returns: The number of variables eliminated, or undefined if skipped.
  run: ->
    # Our optimization does not account for closures.
    if @hasClosures @body then return undefined

    @calculateBasicVarStats()
    @analyzeInitialValues()
    @calculateTransitiveDependencies()
    @analyzeLiveRanges()

    toReplace = {}
    eliminated = 0
    for varName of @isSingleDef
      if @isEliminateable varName
        toReplace[varName] = @initialValue[varName]
        eliminated++

    @removeDeclarations toReplace
    @collapseValues toReplace
    @updateUses toReplace

    return eliminated

  # Determines if a function is Emscripten-generated.
  hasClosures: ->
    closureFound = false

    traverse @body, (node, type) ->
      if type in ['defun', 'function']
        closureFound = true
        return false
      return undefined

    return closureFound

  # Runs the basic variable scan pass. Fills the following member variables:
  #   isSingleDef
  #   useCount
  #   initialValue
  calculateBasicVarStats: ->
    traverse @body, (node, type) =>
      if type is 'var'
        for [varName, varValue] in node[1]
          if not varValue? then varValue = ['name', 'undefined']
          @isSingleDef[varName] = not @isSingleDef.hasOwnProperty varName
          @initialValue[varName] = varValue
          @useCount[varName] = 0
      else if type is 'name'
        varName = node[1]
        if varName of @useCount then @useCount[varName]++
      else if type in ['assign', 'unary-prefix', 'unary-postfix']
        varName = node[2][1]
        if @isSingleDef.hasOwnProperty varName
          @isSingleDef[varName] = false
      return undefined
    return undefined

  # Analyzes the initial values of single-def variables. Requires basic variable
  # stats to have been calculated. Fills the following member variables:
  #   dependsOn
  #   usesOnlySimpleNodes
  #   usesOnlySingleDefs
  analyzeInitialValues: ->
    for varName of @isSingleDef
      if not @isSingleDef[varName] then continue
      @usesOnlySimpleNodes[varName] = true
      @usesOnlySingleDefs[varName] = true
      traverse @initialValue[varName], (node, type) =>
        if type not of SIMPLE_NODES
          @usesOnlySimpleNodes[varName] = false
        else if type is 'name'
          reference = node[1]
          if reference != 'undefined'
            if not @dependsOn[reference]? then @dependsOn[reference] = {}
            @dependsOn[reference][varName] = true
            if not @isSingleDef[reference]
              @usesOnlySingleDefs[varName] = false
        return undefined
    return undefined

  # Updates the dependency graph (@dependsOn) to its transitive closure.
  calculateTransitiveDependencies: ->
    incomplete = true
    while incomplete
      incomplete = false
      for target, sources of @dependsOn
        for source of sources
          for source2 of @dependsOn[source]
            if not @dependsOn[target][source2]
              if not @isSingleDef[target]
                @usesOnlySingleDefs[source2] = false
              @dependsOn[target][source2] = true
              incomplete = true
    return undefined

  # Analyzes the live ranges of single-def single-use variables. Requires
  # dependencies to have been calculated. Fills the following member variables:
  #   depsMutatedInLiveRange
  # TODO: Refactor.
  analyzeLiveRanges: ->
    isLive = {}

    checkForMutations = (node, type) =>
      if type in ['label', 'return', 'break', 'continue', 'call', 'new']
        for varName of isLive
          @depsMutatedInLiveRange[varName] = true
        isLive = {}
      else if type is 'name'
        reference = node[1]
        if @dependsOn[reference]?
          for varName of @dependsOn[reference]
            if isLive[varName]
              @depsMutatedInLiveRange[varName] = true
        if isLive[reference]
          delete isLive[reference]
      return undefined

    traverse @body, (node, type) =>
      if type is 'var'
        for [varName, varValue] in node[1]
          if varValue? then traverse varValue, checkForMutations
          if @isSingleDef[varName] and @useCount[varName] == 1
            isLive[varName] = true
        return node
      else if type is 'stat'
        usedInThisStatement = {}
        hasIndirectAccess = false
        traverse node, (node, type) =>
          if type is 'name'
            usedInThisStatement[node[1]] = true
          else if type in ['sub', 'dot']
            hasIndirectAccess = true
          undefined
        if hasIndirectAccess
          for varName of isLive
            if not usedInThisStatement[varName]
              @depsMutatedInLiveRange[varName] = true
              delete isLive[varName]
      else
        checkForMutations node, type
      return undefined

    return undefined

  # Determines whether a given variable can be safely eliminated. Requires all
  # analysis passes to have been run.
  isEliminateable: (varName) ->
    if @isSingleDef[varName]
      if @useCount[varName] == 0
        return @usesOnlySimpleNodes[varName]
      else if @useCount[varName] == 1
        if @usesOnlySimpleNodes[varName]
          return (@usesOnlySingleDefs[varName] or
                  not @depsMutatedInLiveRange[varName])
      else if @useCount[varName] <= MAX_USES
        return @usesOnlySimpleNodes[varName] and @usesOnlySingleDefs[varName]
    return false

  # Removes all var declarations for the specified variables.
  #   @arg toRemove: An object whose keys are the variable names to remove.
  removeDeclarations: (toRemove) ->
    traverse @body, (node, type) ->
      if type is 'var'
        intactVars = (i for i in node[1] when i[0] not of toRemove)
        if intactVars.length
          node[1] = intactVars
          return node
        else
          return ['toplevel', []]
      return undefined
    return undefined

  # Updates all the values for the given variables to eliminate reference to any
  # of the other variables in the group.
  #   @arg values: A map from variable names to their values as AST expressions.
  collapseValues: (values) ->
    incomplete = true
    while incomplete
      incomplete = false
      for varName, varValue of values
        result = traverse varValue, (node, type) ->
          if type == 'name' and node[1] of values
            if node[1] != varName
              incomplete = true
              return values[node[1]]
          return undefined
        if result? then values[varName] = result
    return undefined

  # Replaces all uses of the specified variables with their respective
  # expressions.
  #   @arg replacements: A map from variable names to AST expressions.
  updateUses: (replacements) ->
    traverse @body, (node, type) ->
      if type is 'name' and node[1] of replacements
        return replacements[node[1]]
      undefined
    return undefined


# The main entry point. Reads JavaScript from stdin, runs the eliminator on each
# function, then writes the optimized result to stdout.
main = ->
  # Get the parse tree.
  src = fs.readFileSync('/dev/stdin').toString()
  ast = uglify.parser.parse src

  # Run the eliminator on all functions.
  traverse ast, (node, type) ->
    if type in ['defun', 'function']
      process.stderr.write (node[1] || '(anonymous)') + '\n'
      eliminated = new Eliminator(node).run()
      if eliminated?
        process.stderr.write "  Eliminated #{eliminated} vars.\n"
      else
        process.stderr.write '  Skipped.\n'
    return undefined

  # Write out the optimized code.
  # NOTE: For large file, can't generate code for the whole file in a single
  #       call due to the v8 memory limit. Writing out root children instead.
  for node in ast[1]
    process.stdout.write uglify.uglify.gen_code node, GEN_OPTIONS
    process.stdout.write '\n'

  return undefined

main()
