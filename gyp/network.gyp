{
    'targets': [{
        'target_name': 'nativenetwork',
        'type': 'static_library',
        'conditions': [
            ['OS=="mac"', {
				'xcode_settings': {
					'OTHER_CFLAGS': [
                        '-fvisibility=hidden'
					],
				},
			}],
            ['OS=="linux"', {
                'cflags': [
                    '-fvisibility=hidden',
                ],
            }]
        ],
        'sources': [
            '<(native_network_path)/native_netlib.c',
            '<(native_network_path)/ape_pool.c',
            '<(native_network_path)/ape_hash.c',
            '<(native_network_path)/ape_http_parser.c',
            '<(native_network_path)/ape_array.c',
            '<(native_network_path)/ape_buffer.c',
            '<(native_network_path)/ape_events.c',
            '<(native_network_path)/ape_event_kqueue.c',
            '<(native_network_path)/ape_event_epoll.c',
            '<(native_network_path)/ape_event_select.c',
            '<(native_network_path)/ape_events_loop.c',
            '<(native_network_path)/ape_socket.c',
            '<(native_network_path)/ape_dns.c',
            '<(native_network_path)/ape_timers.c',
            '<(native_network_path)/ape_timers_next.c',
            '<(native_network_path)/ape_base64.c'
        ],
    }],
}
