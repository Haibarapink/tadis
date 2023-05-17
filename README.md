<!--
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 15:30:27
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-19 10:29:40
 * @FilePath: /tadis/README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->

# TadisSQL - A simple relational database


## Running 
![image](doc/tadis.png)

## Table of Contents

- [Tadis - A simple relational database](#tadis---a-simple-relational-database)
  - [Running](#running)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Install](#Install)
    - [Linux](#linux)
  - [Usage](#usage)
    - [COMMAND](#command)
    - [SQL](#sql)

## Introduction
 Tadis is a relational database and it's my personal project.
 This system was developed for educational purposes and should not be used in production environments.
 Tadis supports poor SQL's grammers.

## Install
  ### Linux

  ```
  > git clone https://github.com/Haibarapink/tadis.git
  > cd tadis
  > mkdir build && cd build
  > cmake ../
  > make 
  ```
  ...

## Usage
### COMMAND
* .quit
* .show tables
### SQL 
* Create table example
```
CREATE TABLE school (name varchar(200), age int);
```
Supports SQL types: VARCHAR(size), CHAR(size), FLOAT, INT.

* Insert example 
```
INSERT INTO school VALUES ('AAU', 55);
```
* Select example
```
SELECT * FROM school;
SELECT school.name , school.age FROM school where age=100, 1>2; 
```
* Drop table example
```
DROP TABLE school;
```

hf :)