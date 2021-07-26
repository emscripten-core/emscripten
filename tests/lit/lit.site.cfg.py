import os

script_dir = os.path.dirname(os.path.abspath(__file__))
config.src_root = os.path.dirname(os.path.dirname(script_dir))

lit_config.load_config(
    config, os.path.join(config.src_root, 'tests', 'lit', 'lit.cfg.py'))
