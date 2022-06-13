# Define a global of type `externref`

.globaltype my_global, externref
my_global:

# Define a function that will call out to JS, passing the
# externref to the JS function

.functype log_externref_js (externref) -> ()

.globl log_externref
log_externref:
  .functype log_externref () -> ()
  global.get my_global
  call log_externref_js
  end_function

# Define a function that will call out to JS to grab an
# externref

.functype get_externref_js () -> (externref)

.globl get_externref
get_externref:
  .functype get_externref () -> ()
  call get_externref_js
  global.set my_global
  end_function
