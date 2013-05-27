extern mod std;

use std::getopts;
use std::net::ip;
use std::net::tcp;
use std::uv_global_loop;

fn print_usage(program: &str, _opts: &[std::getopts::Opt]) {
     io::println(fmt!("Usage: %s [options]", program));
     io::println("--port\tPort to connect on");
     io::println("-h --help\tUsage");
}

fn read_lines(sock: &tcp::TcpSocket,
              extra: &str,
              on_line: &fn(&str) -> bool) -> ~str {
    let buf = match sock.read(0u) {
        result::Ok(buffer) => { str::from_bytes(buffer) }
        result::Err(err) => { fail!(~"failed to read") }
    };
    for str::each_line(buf) |line| { on_line(line); };
    return buf;
}

fn main() {
    let args = os::args();
    let program = copy args[0];
    let opts = ~[
        getopts::optflag("h"), getopts::optflag("help"),
        getopts::optopt("host"),
        getopts::optopt("nick"),
        getopts::optopt("port"),
        getopts::optopt("password")

    ];
    let matches = match getopts::getopts(vec::tail(args), opts) {
        result::Ok(m) => { m }
        result::Err(f) => { fail!(getopts::fail_str(f)) }
    };
    if getopts::opt_present(&matches, "h") || getopts::opt_present(&matches, "help") {
        print_usage(program, opts);
        return;
    }

    let nick = match getopts::opt_maybe_str(&matches, "nick") {
        Some(m) => { m }
        None => { ~"ovanbot" }
    };
    let host = match getopts::opt_maybe_str(&matches, "host") {
        Some(m) => { m }
        None => fail!(~"missing argument --host")
    };
    let port_str = match getopts::opt_maybe_str(&matches, "port") {
        Some(m) => { m }
        None => fail!(~"missing argument --port")
    };
    let port: uint = match int::from_str(port_str) {
        Some(m) => { m as uint }
        None => fail!(~"--port not specified as an int")
    };

    let iotask = uv_global_loop::get();
    let ip_addr = ip::get_addr(host, &iotask);
    let ip_addr = result::unwrap(ip_addr)[0];
    let sock = {
        let conn = tcp::connect(ip_addr, port, &iotask);
        if conn.is_ok() {
            result::unwrap(conn)
        } else {
            fail!(~"failed to connect!")
        }
    };

    let irc_write = |s: &str| {
        io::println(fmt!("-> %s", s));
        sock.write(str::to_bytes(fmt!("%s\r\n", s))); };
    if getopts::opt_present(&matches, "password") {
        irc_write(fmt!("PASS %s", getopts::opt_str(&matches, "password")));
    }
    irc_write(fmt!("NICK %s", nick));
    irc_write(fmt!("USER %s 0 ? * :Ovan Bot", nick));

    let on_line = |s: &str| {
        io::println(s);
        if str::contains(s, ":Welcome") {
            irc_write("JOIN #swolepatrol");
        }
        true
    };
    loop {
        read_lines(&sock, "", on_line);
    }

}