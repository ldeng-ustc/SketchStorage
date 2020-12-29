use sketch;
drop table if exists sketch;
drop table if exists flow;
create table sketch(
    ts_second BIGINT UNSIGNED,
    ts_us BIGINT UNSIGNED,
    src_ip INT UNSIGNED,
    dst_ip INT UNSIGNED,
    src_port SMALLINT UNSIGNED,
    dst_port SMALLINT UNSIGNED,
    protocol TINYINT UNSIGNED,
    start_time DOUBLE,
    end_time DOUBLE,
    pkt_cnt INT UNSIGNED,
    flow_size INT UNSIGNED,
    PRIMARY KEY (ts_second, ts_us, src_ip, dst_ip, src_port, dst_port, protocol)
);

create table flow(
    ts_second BIGINT UNSIGNED,
    ts_us BIGINT UNSIGNED,
    src_ip INT UNSIGNED,
    dst_ip INT UNSIGNED,
    src_port SMALLINT UNSIGNED,
    dst_port SMALLINT UNSIGNED,
    protocol TINYINT UNSIGNED,
    start_time DOUBLE,
    end_time DOUBLE,
    pkt_cnt INT UNSIGNED,
    flow_size INT UNSIGNED,
    PRIMARY KEY (src_ip, dst_ip, src_port, dst_port, protocol, ts_second, ts_us)
);

insert into sketch
    (ts_second, ts_us, src_ip, dst_ip, 
        src_port, dst_port, protocol, 
        start_time, end_time, pkt_cnt, flow_size)
    values
    (1, 2, 3, 4, 5, 6, 7, 1.2, 2.3, 3, 45),
    (2, 1, 3, 4, 5, 6, 7, 1.2, 2.3, 3, 45),
    (2, 1, 4, 3, 5, 6, 7, 1.2, 2.3, 3, 45),
    (1, 2, 4, 3, 5, 6, 7, 1.2, 2.3, 3, 45),
    (1, 2, 3, 4, 7, 5, 8, 1.222, 2.3, 3, 45),
    (1, 2, 3, 4, 7, 5, 7, 1.222, 2.3, 3, 45);

        SELECT 
            src_ip, dst_ip, src_port, dst_port, protocol, 
            min(start_time), max(end_time), sum(pkt_cnt), sum(flow_size)
            FROM sketch 
         WHERE 
            (ts_second > 0 OR (ts_second = 0 AND ts_us >= 0))
             AND 
            (ts_second < 2 OR (ts_second = 2 AND ts_us <= 3))
         GROUP BY 
            src_ip, dst_ip, src_port, dst_port, protocol;
