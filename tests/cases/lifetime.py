if Settings.MICRO_OPTS:
  assert '__stackBase__' in generated, 'There should be some stack activity (without which, we cannot do the next checks)'
  assert '__stackBase__+4' not in generated, 'All variables should have been nativized'
  assert '__stackBase__+8' not in generated, 'All variables should have been nativized'
  assert 'comp_addr' not in generated, 'This variable should have been eliminated during analysis'

