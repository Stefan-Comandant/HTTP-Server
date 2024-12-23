#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <functional>
#include <string>

namespace WebServer {

    struct HTTP_Request;

    // Context that will be passed to each path handler.
    class Context;

    // Handler used when registering paths or middlewares with the router's path registering methods (GET/POST/PATCH...).
    typedef std::function<void(Context)> Handler;

    enum HTTP_METHODS{
        METHOD_INVALID = -1,
        METHOD_GET,
        METHOD_HEAD,
        METHOD_POST,
        METHOD_PUT,
        METHOD_DELETE,
        METHOD_CONNECT,
        METHOD_OPTIONS,
        METHOD_TRACE,
        METHOD_PATCH,
    };

    enum class HTTP_CODES {
        // Informational
        Continue = 100,
        SwitchingProtocols = 101,
        Processing = 102,
        EarlyHints = 103,

        // Success
        OK = 200,
        Created = 201,
        Accepted = 202,
        NonAuthoritativeInformation = 203,
        NoContent = 204,
        ResetContent = 205,
        PartialContent = 206,
        MultiStatus = 207,
        AlreadyReported = 208,
        IMUsed = 226,

        // Redirection
        MultipleChoices = 300,
        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        NotModified = 304,
        TemporaryRedirect = 307,
        PermanentRedirect = 308,

        // Client Error
        BadRequest = 400,
        Unauthorized = 401,
        PaymentRequired = 402,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        NotAcceptable = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeout = 408,
        Conflict = 409,
        Gone = 410,
        LengthRequired = 411,
        PreconditionFailed = 412,
        ContentTooLarge = 413,
        URITooLong = 414,
        UnsupportedMediaType = 415,
        RangeNotSatisfiable = 416,
        ExpectationFailed = 417,
        MisdirectedRequest = 421,
        UnprocessableContent = 422,
        Locked = 423,
        FailedDependency = 424,
        TooEarly = 425,
        UpgradeRequired = 426,
        PreconditionRequired = 428,
        TooManyRequests = 429,
        RequestHeaderFieldsTooLarge = 431,
        UnavailableForLegalReasons = 451,

        // Server Error
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
        HTTPVersionNotSupported = 505,
        VariantAlsoNegotiates = 506,
        InsufficientStorage = 507,
        LoopDetected = 508,
        NetworkAuthenticationRequired = 511,
    };

    struct Path;

    class Router;

    struct FD_Listen_Options;
    class FD_Wrapper;
};

#endif
