#include "NativeNML.h"
#include "NativeJS.h"

#include <string.h>

NativeNML::NativeNML(ape_global *net) :
    net(net), NFIO(NULL), njs(NULL)
{

}

NativeNML::~NativeNML()
{

}

void NativeNML::loadFile(const char *file)
{
    NFIO = new NativeFileIO(file, this, this->net);
    NFIO->open("r");
}

void NativeNML::onAssetsItemReady(NativeAssets::Item *item)
{
    switch(item->fileType) {
        case NativeAssets::Item::ITEM_SCRIPT:
        {
            size_t len = 0;
            const unsigned char *data = item->get(&len);
            njs->LoadScriptContent((const char *)data, len, item->getName());

            break; 
        }
        default:
            break;
    }
}

static void NativeNML_onAssetsItemRead(NativeAssets::Item *item, void *arg)
{
    class NativeNML *nml = (class NativeNML *)arg;

    nml->onAssetsItemReady(item);
}

void NativeNML::loadAssets(rapidxml::xml_node<> &node)
{
    using namespace rapidxml;

    NativeAssets *assets = new NativeAssets(NativeNML_onAssetsItemRead, this);

    for (xml_node<> *child = node.first_node(); child != NULL;
        child = child->next_sibling())
    {
        xml_attribute<> *src = NULL;
        NativeAssets::Item *item = NULL;

        if ((src = child->first_attribute("src"))) {
            xml_attribute<> *id = child->first_attribute("id");
            item = new NativeAssets::Item(src->value(),
                NativeAssets::Item::ITEM_UNKNOWN, net);

            item->setName(id != NULL ? id->value() : src->value());

            assets->addToPendingList(item);
        } else {
            item = new NativeAssets::Item(NULL, NativeAssets::Item::ITEM_UNKNOWN, net);            
            
            item->setName("inline");
            assets->addToPendingList(item);
            item->setContent(child->value(), child->value_size());
        }
        
        if (!strncasecmp(child->name(), "script", 6)) {
            item->fileType = NativeAssets::Item::ITEM_SCRIPT;
            //printf("got a script tag\n");
        }
        //printf("Node : %s\n", child->name());
    }
}


void NativeNML::loadData(char *data, size_t len)
{
    using namespace rapidxml;

    xml_document<> doc;

    printf("Loading data\n");

    try {
        doc.parse<0>(data);
    } catch(rapidxml::parse_error &err) {
        printf("XML error : %s\n", err.what());

        return;
    }

    xml_node<> *node = doc.first_node("application");
    if (node == NULL) {
        printf("XML : <application> node not found\n");
        return;
    }

    for (xml_node<> *child = node->first_node(); child != NULL;
        child = child->next_sibling())
    {
        for (int i = 0; nml_tags[i].str != NULL; i++) {
            if (!strncasecmp(nml_tags[i].str, child->name(),
                child->name_size())) {

                printf("Found tag : %s\n", child->name());

                (this->*nml_tags[i].cb)(*child);
            }
        }
    }

}

void NativeNML::onNFIOOpen(NativeFileIO *NFIO)
{
    NFIO->read(NFIO->filesize);
}

void NativeNML::onNFIORead(NativeFileIO *NFIO, unsigned char *data, size_t len)
{
    this->loadData((char *)data, len);
}
