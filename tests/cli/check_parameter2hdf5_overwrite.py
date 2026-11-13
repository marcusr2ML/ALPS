"""Check that parameter2hdf5 asks before replacing the inputs of a job that has output."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def convert(program, directory, parameters, answer, xml_path):
    (directory / "parm").write_text(parameters)
    environment = dict(os.environ, ALPS_XML_PATH=xml_path)
    return subprocess.run([program, "parm", "job"], cwd=directory, input=answer,
                          capture_output=True, text=True, timeout=30, env=environment)


def main():
    program, xml_path = sys.argv[1], sys.argv[2]
    with tempfile.TemporaryDirectory(prefix="alps parameter2hdf5 ") as name:
        directory = Path(name)
        first = convert(program, directory, "L = 4\n{ T = 1 }\n", "", xml_path)
        assert first.returncode == 0, (first.returncode, first.stderr)
        job = (directory / "job.in.xml").read_text()
        assert '<OUTPUT file="job.out.h5"/>' in job, job
        task_input = (directory / "job.task1.in.h5").read_bytes()

        # The scheduler writes the job output named in job.in.xml.
        (directory / "job.out.h5").write_text("<JOB/>\n")
        second = convert(program, directory, "L = 8\n{ T = 1 }\n", "n\n", xml_path)
        assert second.returncode == 255, (second.returncode, second.stdout, second.stderr)
        assert "job.out.h5" in second.stderr and "Aborted." in second.stderr, second.stderr
        assert (directory / "job.task1.in.h5").read_bytes() == task_input, \
            "job.task1.in.h5 was replaced although the answer was 'n'"


if __name__ == "__main__":
    main()
