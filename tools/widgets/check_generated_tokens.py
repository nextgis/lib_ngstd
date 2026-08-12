"""Check that committed C++ token data matches the canonical JSON."""

import sys
import tempfile
from pathlib import Path

from render_cpp_tokens import CppTokenRenderer
from token_model import TokenModel


class Application:
    @staticmethod
    def run() -> int:
        source_path = Path(sys.argv[1])
        generated_path = Path(sys.argv[2])
        expected = CppTokenRenderer().render(TokenModel.load(source_path))
        actual = generated_path.read_text(encoding="utf-8")
        if actual == expected:
            return 0
        with tempfile.NamedTemporaryFile(
            mode="w",
            suffix=".h",
            encoding="utf-8",
            delete=False,
        ) as temporary_file:
            temporary_file.write(expected)
            temporary_path = temporary_file.name
        print(
            "Generated token table is stale. Expected output: "
            f"{temporary_path}"
        )
        return 1


if __name__ == "__main__":
    raise SystemExit(Application.run())
