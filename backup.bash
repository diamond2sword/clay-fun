#!/bin/bash

#shellcheck ignore=next

CMD_NAME="backup.bash"
CMD_DIR="$HOME/clay-fun"
CMD_PATH="$CMD_DIR/$CMD_NAME"
ROOT_VERSIONS_DIR="$CMD_DIR/versions"

main()
{
	[[ $1 ]] || {
		echo "ERROR: args required"
		exit 1
	}
	[[ $3 ]] && {
		echo "ERROR: too much args"
		exit 1
	}
	project_name="$1"
	project_path="$CMD_DIR/$project_name"
	project_versions_dir="$ROOT_VERSIONS_DIR/$project_name"
	mkdir -p "$project_versions_dir"
	mapfile -t ids < <(find "$project_versions_dir" -mindepth 1 -maxdepth 1 | rev | cut -d '/' -f1 | rev | grep '^[0-9]\+$')
	min_id=$(min 0 "${ids[@]}")
	max_id=$(max 0 "${ids[@]}")
	next_id=$((max_id + 1))

	[[ $2 ]] && (($2 > max_id)) && {
		next_id=$2
	}

	last_backup="$project_versions_dir/$max_id/$project_name-$max_id"
	next_backup="$project_versions_dir/$next_id/$project_name-$next_id"

	echo "cmd: $CMD_NAME, project: $project_name, versions: ($min_id to $max_id), next: $next_id"
	echo_var CMD_PATH project_path next_backup
	echo -n "CONTINUE? [y]:"
	read -r confirm
	[[ "$confirm" == "y" ]] && {
		mkdir -p "$next_backup"
		cp -rf $(find "$project_path" -mindepth 1 -maxdepth 1) "$next_backup"
		echo Done.
		echo -n "DIFF? [y]:"
		read -r confirm
		[[ "$confirm" == "y" ]] && {
			diff_r "$last_backup" "$next_backup"
		}
	}
}

diff_r()
{
	dir1="$1"
	dir2="$2"
	diff_dir="$CMD_DIR/diff.d"
	comm <(find "$dir1" -type f -printf "%P\n" | sort) \
		<(find "$dir2" -type f -printf "%P\n" | sort) |
	while read -r file; do
		file1="$dir1/$file"
		file2="$dir2/$file"
		file_dir="$(dirname "$diff_dir/$file")"
		diff_file="$diff_dir/$file.diff"
		mkdir -p "$file_dir"
		if [[ -f $file1 ]] && [[ -f $file2 ]]; then
			diff --minimal "$file1" "$file2"
		else
			diff --minimal <([[ -f "$file1" ]] && echo "$file1") <([[ -f "$file2" ]] && echo "$file2") 
		fi | bat --tabs 4 --paging=never > "$diff_file"
		bat --tabs 4 --paging=never "$diff_file"
	done
}

echo_var()
{
	[[ $* ]] || exit
	vars=("$@")
	for var in "${vars[@]}"; do
		if [[ $(eval "declare -p $var") =~ "declare -a" ]]; then
			eval echo "$var: \\'\${${var}[@]}\\'"
		else
			eval echo "$var: \\'\$$var\\'"
		fi
	done
}

path_name()
{
	echo "$@" | rev | cut -d '/' -f1 | rev
}

min(){ reduce '((n < final)) && final=$n' "$@"; }
max(){ reduce '((n > final)) && final=$n' "$@"; }

reduce()
{
	operation="$1"; shift
	final="$1"; shift
	[[ $final ]] || {
		echo "Error: requires args"
		exit 1
	}
	numbers=("$@")
	[[ ${numbers[*]} ]] || {
		echo "$final"
		exit
	}
	for n in "${numbers[@]}"; do
		eval "$operation"   
	done
	echo "$final"
}

main "$@"
