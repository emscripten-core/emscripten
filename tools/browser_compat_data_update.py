import json
import requests


out = {}


def parse_version(version, browser):
  version = [int(x) for x in version.split('.')]
  if browser == 'safari' or browser == 'nodejs':
    return sum(x * 100 ** (2 - i) for i, x in enumerate(version))
  else:
    return version[0]


# Traverse the data structure. Find objects that have `__compat` key.
# The `__compat` key has a `support` object, where each key (browser name) is associated with `{version_added: "version"}`.
# For each path during traverse when we find a `__compat` key, we add just the "version" under the same path to the `out` object.
def traverse_bcd(obj):
  out = {}
  for key, value in obj.items():
    if not key.startswith('__'):
      # If it's a feature group, traverse it recursively.
      out[key] = traverse_bcd(value)
    elif key == '__compat':
      # If it's a feature, parse its compat data into just minimum versions of the browsers.
      out_value = {}
      for browser, support in value['support'].items():
        if browser not in ['chrome', 'firefox', 'safari', 'nodejs']:
          # Skip derivative browsers or browsers we don't officially support.
          continue
        # If support is a list, it's a feature history with most relevant entry at the top.
        if type(support) is list:
          support = support[0]
        assert type(support) is dict
        version_added = support['version_added']
        if type(version_added) is bool or version_added == 'preview':
          # We don't know the exact version or it's only available in preview, assume not supported just to be safe.
          continue
        assert type(version_added) is str
        # To be on the safe side, skip if we don't know any keys, not just on `version_removed` and `flags`.
        unknown_keys = set(support.keys()) - {'version_added', 'notes', 'alternative_name'}
        if unknown_keys:
          # Assume unsupported if feature has been removed after it was added, requires flags or something like that.
          continue
        out_value[browser] = parse_version(version_added.lstrip('≤'), browser)
      # Don't bother adding JS features that have always been supported, we're not going to feature-test those.
      if out_value == {'chrome': 1, 'firefox': 1, 'nodejs': 1000, 'safari': 10000}:
        continue
      out['#'] = out_value
  return out


# Pull JavaScript builtins support from MDN's Browser Compat Data.
data = requests.get('https://unpkg.com/@mdn/browser-compat-data/data.json').json()
out['js'] = traverse_bcd(data['javascript']['builtins'])


# BCD data is not as complete for Wasm features as webassembly.org, at least for now.
data = requests.get('https://webassembly.org/features.json').json()
out_wasm = out['wasm'] = {}
for feature in data['features']:
  out_wasm[feature] = {'#': {}}
for browser in ['Chrome', 'Firefox', 'Safari', 'Node.js']:
  normalized_browser = browser.lower().replace('.', '')
  for feature, support in data['browsers'][browser]['features'].items():
    if type(support) is list:
      # If support is a list, first item is the version and 2nd is a text note.
      support = support[0]
    if type(support) is not str or support == 'flag':
      continue
    out_wasm[feature]['#'][normalized_browser] = parse_version(support, normalized_browser)


with open('tools/browser_compat_data.json', 'w') as f:
  json.dump(out, f, indent='\t')
