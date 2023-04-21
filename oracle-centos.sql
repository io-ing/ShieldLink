-- 01 创建表空间
-- 注意表空间的路径 根据实际安装环境进行调整

CREATE TABLESPACE ts_seckey_admin 
    LOGGING  
    DATAFILE '/u01/app/oracle/oradata/orcl/ts_seckeymng_admin.dbf' SIZE 50M 
    EXTENT MANAGEMENT LOCAL;
    

CREATE TABLESPACE ts_seckey_op
    LOGGING 
    DATAFILE '/u01/app/oracle/oradata/orcl/ts_seckeymng_op.dbf' SIZE 200M
    EXTENT MANAGEMENT LOCAL;
    
ALTER DATABASE DATAFILE '/u01/app/oracle/oradata/orcl/ts_seckeymng_admin.dbf' AUTOEXTEND ON NEXT   50M MAXSIZE UNLIMITED;
ALTER DATABASE DATAFILE '/u01/app/oracle/oradata/orcl/ts_seckeymng_op.dbf' AUTOEXTEND ON NEXT  200M MAXSIZE UNLIMITED; 

commit;


-- 02 创建方案 (创建用户)
CREATE USER SECMNG  PROFILE DEFAULT 
    IDENTIFIED BY SECMNG DEFAULT TABLESPACE USERS
    ACCOUNT UNLOCK;

-- 资源和登录权限
GRANT resource TO SECMNG;
GRANT create session TO SECMNG;


-- 创建 网点信息表 --编号 名称 描述 授权码 状态(0可用  1不可用)
CREATE TABLE SECMNG.SECNODE(
	id     				char(4) PRIMARY KEY, 
	name     			VARCHAR2(128) NOT NULL,
 	nodedesc      		VARCHAR2(512),
 	createtime			date,
 	authcode			NUMBER(12),
 	state     			NUMBER(4)
)TABLESPACE ts_seckey_admin;

INSERT INTO SECMNG.secnode VALUES('0001', '网银中心', '北京金融街23号', '15-7月-15', 1, 0);
INSERT INTO SECMNG.secnode VALUES('1111', '广东分中心1111', '广州天河金融', '15-7月-15', 1111, 0);
commit;

-- 创建 网点密钥表, 客户端网点 服务器端网点 密钥号 密钥产生时间 密钥状态 
CREATE TABLE SECMNG.SECKEYINFO(
	clientid          	char(4) constraint secmng_seckeynode_clientid_fk references SECMNG.SECNODE(id),
	serverid          	char(4) constraint secmng_seckeynode_serverid_fk references SECMNG.SECNODE(id),
	keyid            	NUMBER(9) PRIMARY KEY, 
	createtime			date,
	state				NUMBER(4),
	seckey				VARCHAR2(512)
)TABLESPACE ts_seckey_admin;

--创建索引 在新的表空间上
CREATE  INDEX SECMNG.IX_SECKEYINFO_clientid ON SECMNG.SECKEYINFO(clientid) TABLESPACE ts_seckey_admin;

commit;


-- keysn序列号
CREATE TABLE SECMNG.KEYSN(
	ikeysn 		number(12)  PRIMARY KEY
) TABLESPACE ts_seckey_admin;

INSERT INTO SECMNG.KEYSN(ikeysn) VALUES (1);
commit;


CREATE TABLE SECMNG.SRVCFG(
	key				VARCHAR2(64),
	valude			VARCHAR2(128)
)TABLESPACE ts_seckey_admin;
	
-- 创建 交易信息表 交易者 交易时间 交易事件 交易描述 （什么人 在什么时间 干了什么事）
CREATE TABLE SECMNG.TRAN(
	iID					Number(12) PRIMARY KEY,
 	tran_operator     	NUMBER,
 	trantime			date,
 	tranid          	NUMBER(4),
 	trandesc      		VARCHAR2(512)	
)TABLESPACE ts_seckey_op;


-- 创建索引
CREATE INDEX SECMNG.IX_TRAN_tran_operator ON SECMNG.TRAN(tran_operator) TABLESPACE ts_seckey_op;

-- 创建触发器 自增字段
CREATE SEQUENCE SECMNG.SEQiID INCREMENT BY 1 START WITH 1 
    MAXVALUE 1.0E28 MINVALUE 1 NOCYCLE 
    CACHE 20 NOORDER;
    
CREATE OR REPLACE TRIGGER SECMNG.SEQiID
  BEFORE INSERT ON SECMNG.TRAN
    for each row
begin
    select SECMNG.SEQiID.nextval into :new.iID from dual;
end;
/


INSERT INTO SECMNG.TRAN(tran_operator, trantime, tranid, trandesc) VALUES(1, '15-7月-15', 1, '创建网银总节点-测试数据');

commit;

------停止-----
-- 04创建新用户方案 	通过 SECMNGUSER1 来访问数据库, 让管理终端系统用   
CREATE USER "SECMNGADMIN"  PROFILE "DEFAULT" IDENTIFIED BY "123456" DEFAULT TABLESPACE "USERS" ACCOUNT UNLOCK;

GRANT "CONNECT" TO "SECMNGADMIN";
GRANT SELECT ANY TABLE TO "SECMNGADMIN";

GRANT resource TO SECMNGADMIN;
GRANT create session TO SECMNGADMIN;


GRANT DELETE ON SECMNG.SRVCFG TO "SECMNGADMIN";
GRANT INSERT ON SECMNG.SRVCFG TO "SECMNGADMIN";
GRANT UPDATE ON SECMNG.SRVCFG TO "SECMNGADMIN";


GRANT DELETE ON SECMNG.SECNODE TO "SECMNGADMIN";
GRANT INSERT ON SECMNG.SECNODE TO "SECMNGADMIN";
GRANT UPDATE ON SECMNG.SECNODE TO "SECMNGADMIN";

GRANT DELETE ON SECMNG.SECKEYINFO TO "SECMNGADMIN";
GRANT INSERT ON SECMNG.SECKEYINFO TO "SECMNGADMIN";
GRANT UPDATE ON SECMNG.SECKEYINFO TO "SECMNGADMIN";

GRANT DELETE ON SECMNG.TRAN TO "SECMNGADMIN";
GRANT INSERT ON SECMNG.TRAN TO "SECMNGADMIN";
GRANT UPDATE ON SECMNG.TRAN TO "SECMNGADMIN";

commit;

    
 
-- Insert Into SECMNG.SECKEYINFO(clientid, serverid, keyid, createtime, state, seckey)
-- values ('1111', '0001', 1, to_date('2015-07-14 21:09:09', 'yyyy-mm-dd hh24:mi:ss'), 0, 'zzz')  ;
