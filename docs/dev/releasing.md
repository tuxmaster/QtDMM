# Versions and releases

## Version scheme

QtDMM uses calendar versioning ([CalVer](https://calver.org/)) in the form
**YY.N**: the two-digit year, then a running number for the releases of that
year. A version number thus says how current a build is:

| Version | Meaning |
|---|---|
| `26.1` | first release of 2026 (`26.2` the second, `27.1` the first of 2027) |
| `26.1.1` | bugfix release of 26.1 |
| `26.1-rc1` | release candidate 1 of 26.1 (also `-beta1`, `-alpha1`) |
| `26.2-dev.14+g1234abc` | development build: 14 commits after 26.1, heading for 26.2 |
| `26.1-rc1.3+g1234abc` | 3 commits after 26.1-rc1, before 26.1 |

There is no fixed release calendar; N simply counts the releases of the year.

The version comes from **git tags**, and there is nothing to bump by hand.
`cmake/git_version.cmake` runs `git describe` on the newest tag of the form
`YY.N[.P][-rcK]`. The old tags `0.9.x` and `1.0.0-alpha.1` do not match and
are ignored. The same version appears in different forms:

| Where | Form | Example |
|---|---|---|
| UI, `--version`, SCPI `*IDN?` | as above | `26.1-rc1` |
| RPM / DEB | `~` sorts a pre-release before the release | `26.1~rc1`, `26.2~dev.14.g1234abc` |
| Arch (`PKGBUILD`) | VCS form | `26.1rc1.r3.g1234abc` |
| macOS bundle, Windows file version | numbers only | `26.1.0` |
| Windows download names | `+` replaced by `-` | `QtDMM-26.1-rc1-windows-x64.exe` |

Builds without `.git` still know their version:

- The CPack source tarball carries a `.tarball-version` file.
- GitHub's source downloads get it through `export-subst` in `.archive-version`.

A clone needs its tags: `git fetch --tags`. The CI checkouts use `fetch-depth: 0`. `ctest -R version_scheme` checks the mapping from tag to version.

## Making a release

1. **CHANGELOG**: give the top section its version and date,
   `* 12/10/2026 26.1`.
2. **AppStream** (`assets/appimage/qtdmm.appdata.xml`): add the release at
   the top of `<releases>`. Use `type="development"` for a release candidate:

   ```xml
   <releases>
     <release version="26.1" date="2026-10-12"/>
     <release version="26.1-rc1" date="2026-10-01" type="development"/>
   </releases>
   ```

3. Merge that as a normal PR, then tag the merge commit on `master` and push
   the tag:

   ```bash
   git tag -a 26.1-rc1 -m "QtDMM 26.1-rc1"
   git push upstream 26.1-rc1
   ```

4. The **Release** workflow (`.github/workflows/release.yml`) builds the
   Linux AppImage and the Windows installer and ZIP. It then creates a
   **draft** release with them, marked as a pre-release when the tag has a
   suffix.
   If the release was made on GitHub's release page instead (which creates
   the tag), the workflow adds the files to it and leaves it as it is. Its
   builds take about 20 minutes, and until then the release has no binaries.
   Mark a release candidate as a pre-release there by hand.
5. Check the draft on GitHub: download and start the builds, then write the
   notes (from CHANGELOG). Publish it.
6. Tell the website: the download page links the new release.

A bugfix release `26.1.1` is tagged the same way, on a branch `26.1` created
from the `26.1` tag when `master` has moved on.
