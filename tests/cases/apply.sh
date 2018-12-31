# fixups from llvm
for name in "$@"
do
  python3 update.py < "$name" > "$name.tmp" && mv "$name.tmp" "$name"
  rm -f "$name.tmp"
done

