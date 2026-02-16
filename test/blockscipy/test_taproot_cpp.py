import os
import shutil
import subprocess

import pytest


@pytest.mark.btc
def test_taproot_cpp_gtests(chain_name):
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    config_path = os.path.join(repo_root, "test", "blocksci", "config.json")

    binary_candidates = [
        os.path.join(repo_root, "build-dbg", "test", "blocksci", "blocksci_unittest"),
        os.path.join(repo_root, "build", "test", "blocksci", "blocksci_unittest"),
    ]
    binary_path = next((p for p in binary_candidates if os.path.isfile(p)), None)
    if binary_path is None:
        binary_path = shutil.which("blocksci_unittest")

    if binary_path is None:
        pytest.skip("blocksci_unittest not found; build it with: cmake --build build-dbg --target blocksci_unittest")

    if not os.path.isfile(config_path):
        pytest.skip("Missing BlockSci gtest config at {}".format(config_path))

    result = subprocess.run(
        [binary_path, config_path, "--gtest_filter=TaprootAddress.*"],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        pytest.fail(
            "Taproot gtests failed.\nstdout:\n{}\nstderr:\n{}".format(result.stdout, result.stderr)
        )
