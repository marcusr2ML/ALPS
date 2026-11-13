"""Check that parameter2xml and parameter2hdf5 reject a second output basename."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    xml_path, programs = sys.argv[1], sys.argv[2:]
    environment = dict(os.environ, ALPS_XML_PATH=xml_path)
    for program in programs:
        with tempfile.TemporaryDirectory(prefix="alps parameter tools ") as name:
            directory = Path(name)
            (directory / "parm").write_text("L = 4\n{ T = 1 }\n")
            result = subprocess.run([program, "parm", "first", "second"], cwd=directory,
                                    capture_output=True, text=True, timeout=30, env=environment)
            written = sorted(path.name for path in directory.iterdir() if path.name != "parm")
            assert result.returncode != 0, (program, result.returncode, written)
            assert "Usage:" in result.stderr, (program, result.stderr)
            assert written == [], (program, written)


if __name__ == "__main__":
    main()
