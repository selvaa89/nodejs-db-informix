/* tests */

/*
 * Connection configurations
 */
var settings = JSON.parse(require('fs').readFileSync('./tests/db_conf.json','utf8'));

/*
 * Create an Informix database nodejs object
 */
var bindings = require("../nodejs-db-informix");

/*
 * Create a new Informix database bindings. Setup event callbacks. Connect to
 * the database.
 * c - connection
 */
var c = new bindings.Informix(settings);
c.on('error', function(error) {
    console.log("Error: ");
    console.log(error);
}).on('ready', function(server) {
    console.log("Connection ready to ");
    console.log(server);
}).connect(function(err) {
    if (err) {
        throw new Error('Could not connect to DB');
    }
    console.log('Connected to db with ');
    console.log(settings);
    console.log("isConnected() == " + c.isConnected());

    var q, rs;

    // console.log(c);
    // q = "select * from customer order by customer_num";
    /*
    q = "select * from units order by unit_name";
    var rs = this.query(q
        , []
        , {
            start: function(q) {
                console.log(q);
            }
            , async: false
            , cast: false
            , each: function(r) {
                console.log("XXXX");
                console.log(r);
            }
        }).execute();
    */

    /*
    rs = c.query().select("*").from("foo").execute({
        start: function (q) {
            console.log (q);
        }
       , async : false
    });

    console.log('Result set: ');
    console.log(rs);

    */

    rs = this
        .query(
              ""
            , []
            , function () {
                console.log('CALLBACK:');
                console.log(arguments);
            }
            , {
                start: function(q) {
                    console.log('START:');
                    console.log(q);
                }
                , finish: function(f) {
                    console.log('Finish:');
                    console.log(f);
                }
                , async: false
                , cast: true
            }
        )
        .select("*")
        .from("systables", false)
        .where("owner='amitkr'")
        .orderby("tabid")
        .execute();

});



/*
var tests = require("./tests_base.js").get(function(callback) {
    new bindings.Database(settings).connect(function(err) {
        if (err) {
            throw new Error('Could not connect to test DB');
        }
        console.log('Connected to db with ' + settings);
        callback(this);
    });
});

// console.log (tests);

for(var test in tests) {
    console.log(test);
    exports[test] = tests[test];
}
*/
