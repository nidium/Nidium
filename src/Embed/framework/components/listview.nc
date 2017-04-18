<component name="listview">
    <nss>
        list: {
            backgroundColor: "#f8f8f8",
            flexDirection: "column",
            flexGrow:1,
            color: "#d0d0d0",
            fontSize: 16,
            lineHeight: 56,
            textAlign: "left",
            overflow:false,
        },

        icon: {
            flexGrow:1,
            fontSize: 24,
            lineHeight: 24,
            width: 24,
            maxWidth: 24
        },

        media: {
            flexGrow:1,
            color:"#d0d0d0",
            textAlign:"center",
            fontSize: 32,
            lineHeight: 50,
            maxWidth:50,
            minHeight:50
        },

        chevron: {
            flexGrow:1,
            color:"#d8d8d8",
            marginRight:6
        },

        title : {
            flexGrow:4,
            color:"#444444"
        },

        li : {
            height : 56,
            borderColor : "#d0d0d0",
            borderWidth : 0.50
        }
    </nss>
    <layout class="list">
        <slot></slot>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
                window._onmousewheel = (e) => {
                    this.scrollTop += -(2*e.yrel);
                };
            }

            select(id) {
                this.emit("select", id);
            }

            getItemTemplate(item){
                let id = item.id;
                let media = item.media;
                let title = item.title;

                return(`
                    <li id="${id}" class="li" on:mousedown='this.select(${id})'>
                        <icon class="icon media" shape="${media}"></icon>
                        <span class="title">${title}</span>
                        <icon class="icon chevron" shape="fa-chevron-right"></icon>
                    </li>
                `);
            }

            addItem(item) {
                var node = NML.CreateTree(getItemTemplate(item), this, this.shadowRoot);
                console.log(node);
            }

            setItems(list) {
                var nml = [];

                for (var i=0, l=list.length; i<l; i++) {
                    let item = list[i];
                    nml.push(this.getItemTemplate(item));
                }

                NML.CreateTree(nml.join(''), this, this.shadowRoot);
            }
        }
    </script>
</component>
