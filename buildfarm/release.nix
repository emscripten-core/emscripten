{ nixpkgs ? <nixpkgs>
, officialRelease ? false
, emscriptenSrc ? { outPath = ../.; }
}:

let
  jobs = {

    tarball =
      with import nixpkgs { };
      releaseTools.sourceTarball rec {
        name = "emscripten-tarball";
        src = emscriptenSrc;
        version = "";
        versionSuffix =
          if officialRelease then ""
          else if src ? rev then toString src.rev
          else "";
        buildInputs = [];

        distPhase = ''
          runHook preDist

          dir=$(basename $(pwd))
          case $dir in
            (git-export) vcs=git;;
            (hg-archive) vcs=hg;;
            (*) vcs=unk;;
          esac
          cd ..
          ensureDir "$out/tarballs"
          tar --exclude-vcs -caf "$out/tarballs/$vcs-${version}${versionSuffix}.tar.bz2" $dir
          cd -

          runHook postDist
        '';
        inherit officialRelease;
      };

    build = { system ? builtins.currentSystem }:
      with import nixpkgs { inherit system; };
      let
        # Create config file with paths to LLVM, nodejs, java, ...
        settings = writeScript "emscripten-config.py" ''
          # this file is a modified version of emscripten/settings.py which is
          # modified to fit the Nix store.  If you want to override this defaults,
          # make sure to include it by using exec(open(<file>, 'r').read()) and
          # reset the environment variable EM_CONFIG to your own file.
          EMSCRIPTEN_ROOT = '@src@'
          SANITY_FILE = os.environ.get('EM_SANITY_FILE')
          TEMP_DIR = os.environ.get('TMPDIR', '/tmp')

          JRE_PATH = '${jre}/bin/'
          LLVM_ROOT = '${llvm}/bin/'
          CLANG_PATH = '${clang}/bin/'

          # See below for notes on which JS engine(s) you need
          NODE_JS = '${nodejs}/bin/node'
          SPIDERMONKEY_ENGINE = [os.path.expanduser('~/spidermonkey/bin/js'), '-m', '-n']
          V8_ENGINE = os.path.expanduser('~/v8/bin/d8')

          # Pick the JS engine to use for running the compiler.
          COMPILER_ENGINE = NODE_JS
          #COMPILER_ENGINE = V8_ENGINE
          #COMPILER_ENGINE = SPIDERMONKEY_ENGINE

          # All JS engines to use when running the automatic tests.
          JS_ENGINES = [NODE_JS]

          # :NOTE: Workaround removal of trailing spaces from writeTextFile.
          # Otherwise this cause the failure of the sanity checks because they
          # assume that the file end with a new line.
          @newline@
        '';

      in releaseTools.nixBuild {
        name = "emscripten";
        src = jobs.tarball;

        buildInputs = [ python ]; # jre clang llvm nodejs

        buildPhase = ''
          patchShebangs .
        '';

        checkPhase = ''
          TESTDIR=$(dirname $(pwd))
          export EM_CONFIG=$TESTDIR/emscripten.cfg
          substitute "${settings}" $EM_CONFIG \
            --replace '@src@' "$(pwd)" \
            --replace '@newline@' ""

          export EM_SANITY_FILE=$TESTDIR/sanity.file
          touch $EM_SANITY_FILE

          export EM_CACHE_DIR=$TESTDIR/emscripten_cache

          echo tests/runner.py sanity
          python tests/runner.py sanity

          echo tests/runner.py test_hello_world
          python tests/runner.py test_hello_world
        '';

        installPhase = ''
          # create wrappers to emscripten sources.
          wrapDefaultConfig () {
            echo > $2  "#! $SHELL -e
          : \''${EM_CONFIG=$out/etc/emscripten.cfg}
          export EM_CONFIG
          exec ${python}/bin/python $1 \$@
          "
            chmod a+x $2
          }

          cp -r $sourceRoot $out/src

          ensureDir $out/etc
          substitute ${settings} $out/etc/emscripten.cfg \
            --replace '@src@' '$out/src' \
            --replace '@newline@' ""

          ensureDir $out/bin
          wrapDefaultConfig $out/src/emcc $out/bin/gcc
          wrapDefaultConfig $out/src/em++ $out/bin/g++
          wrapDefaultConfig $out/src/emld $out/bin/ld
          wrapDefaultConfig $out/src/emranlib $out/bin/ranlib
        '';

        meta = {
          description = "LLVM-to-JavaScript compiler";
          homepage = https://github.com/kripken/emscripten/wiki;
          license = lib.licenses.mit;
          longDescription = ''
            Emscripten is an LLVM-to-JavaScript compiler. It takes LLVM bitcode
            (which can be generated from C/C++ using Clang, or any other language
            that can be converted into LLVM bitcode) and compiles that into
            JavaScript, which can be run on the web (or anywhere else JavaScript
            can run).
          '';
          schedulingPriority = "100";
        };
      };

    };
in
  jobs
