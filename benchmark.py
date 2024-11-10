from pathlib import Path
import glob
import argparse
import subprocess


# Params
cutechess_cli_path = Path(r'D:\Downloads\cutechess-1.3.1-win64\cutechess-cli.exe')
project_dir = Path(r'D:\Work\munchess\munchess')
engine_exec_stem = 'munchess'

# Derived
latest_engine_path = project_dir / 'build' / 'Release' / f'{engine_exec_stem}.exe'
hall_of_fame_directory_path = project_dir / 'hall-of-fame'
benchmark_dir = project_dir / 'benchmark'


def get_engine_choices():
    choices = {
        'latest': str(latest_engine_path),
        'l': str(latest_engine_path),
    }
    version_paths = glob.glob(str(hall_of_fame_directory_path / f'{engine_exec_stem}-v*.exe'))
    for version_path in version_paths:
        version_path = Path(version_path)
        stem = version_path.stem
        version_name = stem[len(engine_exec_stem) + 1:]
        choices[version_name] = str(version_path)

    return choices


def main():
    engine_choices = get_engine_choices()

    parser = argparse.ArgumentParser(description='Chess engine benchmark')
    parser.add_argument('e1', choices=engine_choices.keys(), help='Engine 1 version')
    parser.add_argument('e2', choices=engine_choices.keys(), help='Engine 2 version')
    parser.add_argument('n', type=int, default=10, nargs='?', help='Number of games')
    # parser.add_argument('--clear_logs', default=False, action='store_true', help='Clear logs of all engines')
    args = parser.parse_args()

    if args.e1 == 'l':
        args.e1 = 'latest'
    if args.e2 == 'l':
        args.e2 = 'latest'

    e1_dir = benchmark_dir / args.e1 / '1'
    e2_dir = benchmark_dir / args.e2 / '2'
    e1_dir.mkdir(parents=True, exist_ok=True)
    e2_dir.mkdir(parents=True, exist_ok=True)
    
    pgnout_path = benchmark_dir / 'games.pgn'

    command = f'{cutechess_cli_path} -engine cmd="{engine_choices[args.e1]}" ' \
            f'dir="{str(e1_dir)}" -engine cmd="{engine_choices[args.e2]}" dir="{str(e2_dir)}" '\
            f'-each proto=uci tc=inf -rounds {args.n} -pgnout "{str(pgnout_path)}"'

    subprocess.run(command, shell=True)


if __name__ == '__main__':
    main()
