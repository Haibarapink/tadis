<!--
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 15:30:27
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 03:27:36
 * @FilePath: /tadis/README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->

# Tadis - A simple relational database
![image](doc/tadis.png)

## Table of Contents

- [Instaroduction](#Introduction)
- [Install](#Install)
- [Usage](#Usage)
- [Contributing](#contributing)
- [License](#license)

## Introduction
 Tadis is a relational database and it's my personal project.
 This system was developed for educational purposes and should not be used in production environments.
 Tadis supports poor SQL's grammers.

## Install
  Make sure your computer already installed the [boost library](https://www.boost.org/)
  
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
### SQL 
* Create table example
```
CREATE TABLE school (name varchar(200), age int);
```
Tadis supports these SQL datatypes: VARCHAR(size), CHAR(size), FLOAT, INT.

* Insert example 
```
INSERT INTO school VALUES ('AAU', 55);
```


* Delete example
```
TODO:
DELETE * FROM school;
DELETE FROM school;
DELETE FROM school WHERE school.name='HKU';
```
* Select example
```
SELECT * FROM school;
TODO:
SELECT school.name , school.age FROM SCHOOL where age=100; 
```
