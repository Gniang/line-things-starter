const request = require('request');


/**
 * Responds to any HTTP request.
 *
 * @param {!express:Request} req HTTP request context.
 * @param {!express:Response} res HTTP response context.
 */
exports.sendRireki = (req, res) => {
  const user = req.query['user'];
  const usb_id = req.query['usb_id'];

    let data = {
        'app': 1,
        'record': {
            'usb_id': {
                'value': usb_id
            },          
            'user': {
                'value': user
            }
        }
    };

  let params = {
    url: 'https://devxnofri.cybozu.com/k/v1/record.json',
    method: 'POST',
    json: true,
    headers: {
            'Content-Type': 'application/json',
            'X-Cybozu-API-Token': 'mkLMAXzG2gLW5Ktvcd6Z9muO0CZbzDZBNH8XBLwi',
    },
    body: data,
  };
  
  request(params, function(err, resp, body) {
    if (err) {
      console.log(err);
      return;
    }
    console.log(body);
  });

  res.status(200).send(message);
};

