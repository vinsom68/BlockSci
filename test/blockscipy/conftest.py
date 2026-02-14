import pytest
import subprocess
import os
import json


REGTEST_MAGIC = b"\xfa\xbf\xb5\xda"


def _disk_dir(self_dir, chain_name):
    env_key = "BLOCKSCI_{}_DISK_DIR".format(chain_name.upper())
    override = os.environ.get(env_key)
    if override:
        return override
    return "{}/../files/{}/regtest/".format(self_dir, chain_name)


def _read_block_magic(disk_dir, chain_name):
    blk_path = os.path.join(disk_dir, "blocks", "blk00000.dat")
    if not os.path.exists(blk_path):
        raise FileNotFoundError("Missing blk00000.dat for {} at {}".format(chain_name, blk_path))
    with open(blk_path, "rb") as f:
        magic_bytes = f.read(4)
    return magic_bytes


def _maybe_patch_block_magic(config_path, magic_bytes):
    if magic_bytes == REGTEST_MAGIC:
        return
    magic_int = int.from_bytes(magic_bytes, byteorder="little", signed=False)
    with open(config_path, "r") as f:
        conf = json.load(f)
    parser_cfg = conf.get("parser", {})
    disk_cfg = parser_cfg.get("disk", {})
    disk_cfg["blockMagic"] = magic_int
    parser_cfg["disk"] = disk_cfg
    conf["parser"] = parser_cfg
    with open(config_path, "w") as f:
        json.dump(conf, f, indent=4, sort_keys=True)


def pytest_addoption(parser):
    parser.addoption("--btc", action="store_true", help="Run tests for Bitcoin")
    parser.addoption("--bch", action="store_true", help="Run tests for Bitcoin Cash")
    parser.addoption("--ltc", action="store_true", help="Run tests for Litecoin")


def pytest_generate_tests(metafunc):
    metafunc.fixturenames.append("chain_name")
    chains = []
    if metafunc.config.option.btc:
        chains += ["btc"]
    if metafunc.config.option.bch:
        chains += ["bch"]
    if metafunc.config.option.ltc:
        chains += ["ltc"]
    if not chains:
        chains = ["btc", "bch", "ltc"]
    metafunc.parametrize("chain_name", chains, scope="session")


def pytest_runtest_call(item):
    markers = [x.name for x in item.iter_markers()]
    if markers:
        if item.funcargs["chain_name"] not in markers:
            pytest.skip(
                "Skipping test for chain {}".format(item.funcargs["chain_name"])
            )


@pytest.fixture(scope="session")
def chain(tmpdir_factory, chain_name):
    temp_dir = tmpdir_factory.mktemp(chain_name)
    chain_dir = str(temp_dir)
    self_dir = os.path.dirname(os.path.realpath(__file__))

    if chain_name == "btc":
        blocksci_chain_name = "bitcoin_regtest"
    elif chain_name == "bch":
        blocksci_chain_name = "bitcoin_cash_regtest"
    elif chain_name == "ltc":
        blocksci_chain_name = "litecoin_regtest"
    else:
        raise ValueError("Invalid chain name {}".format(chain_name))

    disk_dir = _disk_dir(self_dir, chain_name)

    create_config_cmd = [
        "blocksci_parser",
        chain_dir + "/config.json",
        "generate-config",
        blocksci_chain_name,
        chain_dir,
        "--disk",
        disk_dir,
        "--max-block",
        "100",
    ]
    parse_cmd = ["blocksci_parser", chain_dir + "/config.json", "update"]

    # Parse the chain up to block 100 only
    subprocess.run(create_config_cmd, check=True)
    magic_bytes = _read_block_magic(disk_dir, chain_name)
    _maybe_patch_block_magic(chain_dir + "/config.json", magic_bytes)
    subprocess.run(parse_cmd, check=True)

    # Now parse the remainder of the chain
    subprocess.run(create_config_cmd[:-2], check=True)
    subprocess.run(parse_cmd, check=True)

    import blocksci
    chain = blocksci.Blockchain(chain_dir + "/config.json")
    return chain


@pytest.fixture
def json_data(chain_name):
    import json

    with open("../files/{}/output.json".format(chain_name), "r") as f:
        return json.load(f)
