namespace FNX {
namespace App {

/// A remote compilation server application.
///
/// It accepts build, doc and format commands along with single or
/// multiple source files. Included paths shall be taken in
/// consideration on the remote, though. It practically wraps AOT,
/// Doc and Fmt apps and handles panics.
///
/// It uses gRPC is used for async communication. For authentication,
/// a server instance may be set up with an SQLite URI.
///
/// TODO: A user has its own space on the remote, which they have FTP
/// access to. Can also send w/ `fnx remote -u user - ppass upload
/// my.h include/my.h`.
namespace Remote {

// DANGER: Only listed envars are passed to the server!
//
// ```
// fnx remote \
//   -v \             # Verbose server connection
//   -H example.com \ # Host
//   -P 3000 \        # Port
//   -u user \        # Auth user
//   -p password \    # Auth password (can be stored?)
//   -W my/proj \     # User-specific working directory
//   -j3 \            # Would not exceed server's limit
//   build \          # Command
//   -v \             # Verbose compilation
//   -e FOO=Bar \     # Other envars are not exposed to the server!
//   -o[-p3 -s2] \
//   -c[-wgnu -s11] \
//   -Imy/include     # Can access user-specific dirs
//   main.nx          # File
// ```
class Client;

// DANGER: Must whitelist environment variables seen to compilation
// contexts!
//
// DANGER: Included directories are write-protected, thus can not be
// changed from Lua.
//
// ```
// fnx remote server \
//   -j4 \
//   -I/usr/include \
//   -p 3000 \
//   --db sqlite:// \
// ```
class Server;

}; // namespace Remote

} // namespace App
} // namespace FNX
