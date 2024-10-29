/* eslint-disable no-undef */
db.createUser({
    user: 'intercom',
    pwd: 'intercom',
    roles: [
        {
            role: 'readWrite',
            db: 'intercom',
        },
    ],
});
db.createCollection('test');
