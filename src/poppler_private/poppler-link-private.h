#ifndef POPPLER_LINK_PRIVATE_H
#define POPPLER_LINK_PRIVATE_H

#include "poppler_private_export.h"
#include <memory>
#include <vector>

class Array;
class Dict;
class Object;
class GooString;

class POPPLER_PRIVATE_EXPORT LinkAction
{
public:
    virtual ~LinkAction();
    virtual bool isOk() const = 0;
    virtual int getKind() const = 0;

    enum LinkActionKind {
        actionGoTo,
        actionGoToR,
        actionLaunch,
        actionURI,
        actionNamed,
        actionMovie,
        actionRendition,
        actionSound,
        actionJavaScript,
        actionOCGState,
        actionHide,
        actionResetForm,
        actionSubmitForm,
        actionUnknown
    };
};

class POPPLER_PRIVATE_EXPORT LinkDest
{
public:
    explicit LinkDest(const Array &a);
    bool isOk() const { return ok; }

    enum LinkDestKind {
        destXYZ, destFit, destFitH, destFitV,
        destFitR, destFitB, destFitBH, destFitBV
    };

    LinkDestKind getKind() const { return kind; }
    int getPageNum() const { return pageNum; }
    double getLeft() const { return left; }
    double getBottom() const { return bottom; }
    double getRight() const { return right; }
    double getTop() const { return top; }
    double getZoom() const { return zoom; }

private:
    LinkDestKind kind;
    bool pageIsRef;
    union {
        int pageNum;
    };
    double left, bottom, right, top, zoom;
    bool changeLeft, changeTop, changeZoom;
    bool ok;
};

class POPPLER_PRIVATE_EXPORT LinkGoTo : public LinkAction
{
public:
    explicit LinkGoTo(const Object *destObj);
    ~LinkGoTo() override;
    bool isOk() const override;
    int getKind() const override { return actionGoTo; }
    const LinkDest *getDest() const { return dest.get(); }

private:
    std::unique_ptr<LinkDest> dest;
};

#endif // POPPLER_LINK_PRIVATE_H
