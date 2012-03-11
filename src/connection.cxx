#include "connection.h"

nodejs_db_informix::Connection::Connection()
    : compress(false),
      readTimeout(0),
      reconnect(true),
      sslVerifyServer(false),
      timeout(0),
      writeTimeout(0)
{
    this->port = 0;

    // construct connection from environment variables
    this->connection = new ITConnection();
}

nodejs_db_informix::Connection::Connection(
        std::string h,
        std::string u,
        std::string pw,
        std::string d,
        uint32_t p
) {
    this->setHostname(h);
    this->setUser(u);
    this->setPassword(pw);
    this->setDatabase(d);
    this->setPort(p);
}

nodejs_db_informix::Connection::~Connection() {
    this->close();
}

void nodejs_db_informix::Connection::setCharset(const std::string& charset) throw() {
    this->charset = charset;
}

void nodejs_db_informix::Connection::setCompress(const bool compress) throw() {
    this->compress = compress;
}

void nodejs_db_informix::Connection::setInitCommand(const std::string& initCommand) throw() {
    this->initCommand = initCommand;
}

void nodejs_db_informix::Connection::setReadTimeout(const uint32_t readTimeout) throw() {
    this->readTimeout = readTimeout;
}

void nodejs_db_informix::Connection::setReconnect(const bool reconnect) throw() {
    this->reconnect = reconnect;
}

void nodejs_db_informix::Connection::setSocket(const std::string& socket) throw() {
    this->socket = socket;
}

void nodejs_db_informix::Connection::setSslVerifyServer(const bool sslVerifyServer) throw() {
    this->sslVerifyServer = sslVerifyServer;
}

void nodejs_db_informix::Connection::setTimeout(const uint32_t timeout) throw() {
    this->timeout = timeout;
}

void nodejs_db_informix::Connection::setWriteTimeout(const uint32_t writeTimeout) throw() {
    this->writeTimeout = writeTimeout;
}

bool nodejs_db_informix::Connection::isAlive() throw() {
    if (this->alive) {
        this->alive = this->connection->IsOpen();
    }
    return this->alive;
}

/**
 * XXX: do we really need to pass dbInfo?
 */
bool
nodejs_db_informix::Connection::_prepareITDBInfo(ITDBInfo& dbInfo) {
    // dbInfo = new ITDBInfo();
    
    // setup any custom parameters passed
    std::string db = this->getDatabase();
    if (db.c_str()
            && !db.empty()
            && !dbInfo.SetDatabase(ITString(db.c_str()))) {
        std::cerr << "Failed to set database " << db << std::endl;
    }

    std::string u = this->getUser();
    if (u.c_str()
            && !u.empty()
            && !dbInfo.SetUser(ITString(u.c_str()))) {
        std::cerr << "Failed to set username " << u << std::endl;
    }

    std::string h = this->getHostname();
    if (h.c_str()
            && !h.empty()
            && !dbInfo.SetSystem(ITString(h.c_str()))) {
        std::cerr << "Failed to set hostname " << h << std::endl;
    }

    std::string pw = this->getPassword();
    if (pw.c_str()
            && !pw.empty()
            && !dbInfo.SetPassword(ITString(pw.c_str()))) {
        std::cerr << "Failed to set password " << pw << std::endl;
    }

    return true;
}

void
nodejs_db_informix::Connection::open() throw(nodejs_db::Exception&) {
    // close connection if any
    // and don't worry about the return of this close
    this->close();

    if (this->connection->IsOpen()) {
        throw nodejs_db::Exception("Database connection is alreay open");
    }

    ITDBInfo dbInfo = this->connection->GetDBInfo();

    if (dbInfo.Frozen()) {
        throw nodejs_db::Exception("Database connection is alreay open");
    }

    if (dbInfo.GetSystem().IsNull()) {
        throw nodejs_db::Exception("Which system to connect to?");
    }

    if (dbInfo.GetDatabase().IsNull()) {
        throw nodejs_db::Exception("No database name specified");
    }

    if (dbInfo.GetUser().IsNull()) {
        throw nodejs_db::Exception("No database username specified");
    }

    // prepare dbInfo
    if (!this->_prepareITDBInfo(dbInfo)) {
        throw nodejs_db::Exception("Could not prepare ITDBInfo");
    }

    std::cout << "Connecting with " << std::endl
        << "User: " << dbInfo.GetUser().Data() << std::endl
        << "System: " << dbInfo.GetSystem().Data() << std::endl
        << "Database: " << dbInfo.GetDatabase().Data() << std::endl
        ;

    // setup the dbInfo
    if (!this->connection->SetDBInfo(dbInfo)) {
        throw nodejs_db::Exception("Could not set the ITDBINfo");
    }

    // open connection
    if (!(this->alive = this->connection->Open())) {
        throw nodejs_db::Exception("Connection failed!");
    }

    // check if everything went ok.
    if (!this->connection->IsOpen()) {
        this->alive = false;
        throw nodejs_db::Exception("Cannot create Informix connection");
    }
}

void
nodejs_db_informix::Connection::close() {
    if (this->alive) {
        if(this->connection->Close()) {
            this->alive = false;
        }
    }
    this->alive = false;
}

std::string
nodejs_db_informix::Connection::escape(const std::string& string) const throw(nodejs_db::Exception&) {
    /*
    char* buffer = new char[string.length() * 2 + 1];
    if (buffer == NULL) {
        throw nodejs_db::Exception("Can\'t create buffer to escape string");
    }

    informix_real_escape_string(this->connection, buffer, string.c_str(), string.length());

    std::string escaped = buffer;
    delete [] buffer;
    return escaped;
    */

    return std::string(NULL);
}

std::string
nodejs_db_informix::Connection::version() const {
    /*
    std::string version = informix_get_server_info(this->connection);
    return version;
    */
    return std::string();
}

ITCallbackResult
// nodejs_db_informix::Connection::_QueryErrorHandler(
_QueryErrorHandler(
    const ITErrorManager& err
    , void* args
    , long errCode
) {
    std::ostream *s = (std::ostream*) args;
    (*s) << "Query: [" << errCode << "]"
        << err.SqlState().Data() << ' '
        << err.ErrorText().Data()
        << std::endl;

    return IT_NOTHANDLED;
}

nodejs_db::Result*
nodejs_db_informix::Connection::query(const std::string& query) const throw(nodejs_db::Exception&) {

    ITQuery q(*(this->connection));

    q.AddCallback(_QueryErrorHandler, (void*) &std::cerr);

    ITSet *rs = q.ExecToSet(query.c_str());

    if (q.RowCount() <= 0) {
        if (q.Warn()) {
            throw nodejs_db::Exception(
                    std::string(q.SqlState())
                    + ": "
                    + std::string(q.WarningText())
            );
        }

        if (q.Error()) {
            throw nodejs_db::Exception(
                    std::string(q.SqlState())
                    + ": "
                    + std::string(q.ErrorText())
            );
        }

        throw nodejs_db::Exception("Could not execute query");
    }

    return new nodejs_db_informix::Result(rs);
}

/*
nodejs_db::Result*
nodejs_db_informix::Connection::query(const std::string& query) const throw(nodejs_db::Exception&) {
    return new Result();
}
*/
